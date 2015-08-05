/**
   \file clock.cpp
   \copyright 2014 Jesse Haber-Kucharsky   

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "clock.hpp"

namespace nebula {

static const std::unordered_map<Word, ClockOperation> kOperationMap{
    {0u, ClockOperation::SetDivider}, {1u, ClockOperation::StoreElapsed},
    {2u, ClockOperation::EnableInterrupts}};

void Clock::handleInterrupt(ClockOperation operation,
                            ProcessorState& processor_state) {
  switch (operation) {
    case ClockOperation::SetDivider: {
      LOG(CLOCK, info) << "'SetDivider'";
      Word b = processor_state.read(Register::B);

      if (b != 0u) {
        Word divider = b >= 60u ? 60 : b;

        LOG(CLOCK, info) << format("Turning on with divider 0x%04x.") % divider;

        state_.is_on = true;
        state_.divider = divider;
        state_.elapsed_ticks = 0u;
      } else {
        LOG(CLOCK, info) << "Turning off.";
        state_.is_on = false;
      }

    } break;

    case ClockOperation::StoreElapsed: {
      LOG(CLOCK, info) << "'StoreElapsed'";
      processor_state.write(Register::C, state_.elapsed_ticks);
    } break;

    case ClockOperation::EnableInterrupts: {
      LOG(CLOCK, info) << "'EnableInterrupts'";
      Word b = processor_state.read(Register::B);

      if (b != 0u) {
        LOG(CLOCK, info) << "Turning on interrupts.";
        state_.are_interrupts_enabled = true;
        state_.interrupt_message = b;
      } else {
        LOG(CLOCK, info) << "Turning interrupts off.";
        state_.are_interrupts_enabled = false;
      }
    } break;
  }
}

unique_ptr<ClockState> Clock::start() {
  notify();

  LOG(CLOCK, info) << "Started.";

  while (status() == SimulationStatus::Running) {
    if (!state_.is_on) {
      // Do nothing until the clock is triggered by the computer, or the
      // computer dies.
      LOG(CLOCK, info) << "Off and sleeping. Waiting for interrupt.";
      interrupt_sink_.waitForTriggerOrDeath(*this);
      LOG(CLOCK, info) << "Woken.";

      if (status() != SimulationStatus::Running) {
        break;
      }
    }

    auto now = std::chrono::system_clock::now();

    if (interrupt_sink_.isActive()) {
      LOG(CLOCK, info) << "Got interrupt.";

      auto& processor_state = interrupt_sink_.processorState();
      auto a = processor_state.read(Register::A);
      auto operation = get(a, kOperationMap);

      if (operation) {
        handleInterrupt(*operation, processor_state);
      }

      interrupt_sink_.respond();
      LOG(CLOCK, info) << "Finished handling interrupt.";
    }

    std::this_thread::sleep_until(now + (kClockBasePeriod * state_.divider));

    LOG(CLOCK, info) << "Tick.";
    state_.elapsed_ticks += 1u;

    if (state_.are_interrupts_enabled) {
      computer_->interruptQueue().push(state_.interrupt_message);
    }
  }

  LOG(CLOCK, info) << "Shutting down.";
  return make_unique<ClockState>(state_);
}

}  // namespace nebula
