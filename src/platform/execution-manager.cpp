/**
   \file execution-manager.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "lambda-visitor.hpp"
#include "execution-manager.hpp"

namespace nebula {

ExecutionManager::ExecutionManager(std::shared_ptr<Computer> computer,
                                   const ExecutionManagerOptions& options)
    : Simulation<ProcessorState>{}, computer_{computer}, options_{options} {}

struct CommandVisitor : public boost::static_visitor<void> {
  Computer& computer;
  ExecutionManager& execution_manager;

  explicit CommandVisitor(Computer& computer,
                          ExecutionManager& execution_manager)
      : computer(computer), execution_manager(execution_manager) {}

  // Default is nothing.
  template <typename T>
  void operator()(const T&) const {}

  void operator()(const interactive::ShowBreak&) const {
    for (const auto& pair : execution_manager.break_points_) {
      const auto& break_point = pair.second;
      std::cout << format("%3u at 0x%04x\n") % break_point.index() %
                       break_point.offset();
    }
  }

  void operator()(const interactive::ShowState&) const {
    std::cout << computer;
  }

  void operator()(const interactive::BreakPoint& break_point) const {
    execution_manager.break_points_.emplace(
        std::make_pair(break_point.offset(), break_point));

    std::cout << format("Added break-point %u at 0x%04x.") %
                     break_point.index() % break_point.offset() << std::endl;
  }

  void operator()(const interactive::ShowSource& show_source) const {
    auto instructions = computer.lookAhead(show_source.num_instructions);

    for (const auto& pair : instructions) {
      Word pc = std::get<0>(pair);
      const auto& instruction = std::get<1>(pair);

      std::cout << format("[0x%04x]  ") % pc;

      if (!instruction) {
        std::cout << "(bad instruction)" << std::endl;
      } else {
        std::cout << *instruction << std::endl;
      }
    }
  }
};

void ExecutionManager::enterHaltedLoop(bool* is_termination_requested) {
  bool do_leave_loop = false;
  *is_termination_requested = false;

  auto termination_command_visitor = detail::makeLambdaVisitor<void>(
      [&](const interactive::ConditionalHalt& conditional_halt) {
        halt_condition_ = conditional_halt;
        do_leave_loop = true;
      },
      [](const interactive::BreakPoint&) {},
      [](const interactive::ShowBreak&) {},
      [](const interactive::ShowSource&) {},
      [](const interactive::ShowState&) {},
      [&](const interactive::Quit&) {
        do_leave_loop = true;
        *is_termination_requested = true;

        stop();
      });

  while (!do_leave_loop) {
    auto command = interactive::waitForCommand(computer_->state());

    boost::apply_visitor(CommandVisitor{*computer_, *this}, command);
    boost::apply_visitor(termination_command_visitor, command);
  }
}

std::unique_ptr<ProcessorState> ExecutionManager::start() {
  notify();

  bool do_enter_halted_loop = false;
  bool is_termination_requested = false;

  while (status() == SimulationStatus::Running) {
    auto now = std::chrono::system_clock::now();

    if (options_.do_initial_halt_) {
      options_.do_initial_halt_ = false;
      do_enter_halted_loop = true;
    } else if (halt_condition_ && halt_condition_->doHalt(*computer_)) {
      halt_condition_ = boost::none;
      do_enter_halted_loop = true;
    } else {
      auto pc = computer_->state().read(Special::Pc);
      auto pair = break_points_.find(pc);

      if (pair != break_points_.end()) {
        std::cout << format("Halted at break-point %u.\n") %
                         pair->second.index();

        do_enter_halted_loop = true;
      }
    }

    if (do_enter_halted_loop) {
      enterHaltedLoop(&is_termination_requested);
      do_enter_halted_loop = false;

      if (is_termination_requested) {
        break;
      }
    }

    computer_->step();

    auto target_sleep_period = computer_->state().cycleCount() * options_.clock_period_;
    auto elapsed_time = std::chrono::system_clock::now() - now;

    if (target_sleep_period > elapsed_time) {
      std::this_thread::sleep_for(target_sleep_period - elapsed_time);
    }

    computer_->state_->clearCycleCount();

    if (computer_->state().read(Flag::Aborted)) {
      break;
    }
  }

  return std::unique_ptr<ProcessorState>(
      new ProcessorState{computer_->state()});
}

}  // namespace nebula
