/**
   \file execution-manager.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Manage execution of the computer.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   Just like the hardware devices are each a nebula::Simulation, the
   nebula::Computer itself is controlled by a nebula::ExecutionManager
   nebula::Simulation.

   It is through the execution manager that the logical cycle count is
   translated into a real time delay which should roughly simulated the real
   execution speed of the processor.
 */

#ifndef NEBULA_EXECUTION_MANAGER_HPP_
#define NEBULA_EXECUTION_MANAGER_HPP_

#pragma once

#include "prelude.hpp"
#include "computer.hpp"
#include "interactive.hpp"

#include <unordered_map>

namespace nebula {

/**
   \brief The clock period is measured in nanoseconds.
 */
using ProcessorClockPeriod = std::chrono::nanoseconds;

/**
   \brief Options influencing the ExecutionManager.
 */
class ExecutionManagerOptions {
  friend class ExecutionManager;

 private:
  bool do_initial_halt_{false};
  ProcessorClockPeriod clock_period_{10000};

 public:
  inline explicit ExecutionManagerOptions() {}

  /**
     \brief Present an interactive prompt prior to any execution.

     The default value is \p false.
   */
  inline ExecutionManagerOptions& doInitialHalt(bool value) {
    do_initial_halt_ = value;
    return *this;
  }

  /**
     \brief Set the execution characteristics of the processor.

     The default value ProcessorFreeRun.
   */
  inline ExecutionManagerOptions& clockPeriod(ProcessorClockPeriod value) {
    clock_period_ = value;
    return *this;
  }
};

/**
   \brief Manage the execution of a Computer.
 */
class ExecutionManager : public Simulation<ProcessorState> {
 private:
  std::shared_ptr<Computer> computer_;
  ExecutionManagerOptions options_;

  optional<interactive::ConditionalHalt> halt_condition_{};
  std::unordered_map<Word, interactive::BreakPoint> break_points_{};

  void enterHaltedLoop(bool* is_termination_requested);

 public:
  explicit ExecutionManager(std::shared_ptr<Computer> computer,
                            const ExecutionManagerOptions& options);

  std::unique_ptr<ProcessorState> start() override;

  friend struct CommandVisitor;
};

}  // namespace nebula

#endif  // NEBULA_EXECUTION_MANAGER_HPP_
