/**
   \file keyboard.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "keyboard.hpp"

namespace nebula {

void KeyboardState::setKey(Word key) {
  LOG(KEYBOARD, info) << format("Got <%u>") % key;

  std::lock_guard<std::mutex> lock{mutex_};
  key_ = key;
  was_interrupt_sent_ = false;
}

static const std::unordered_map<Word, KeyboardOperation> kOperationMap{
    {0u, KeyboardOperation::Clear},
    {1u, KeyboardOperation::Store},
    {2u, KeyboardOperation::Query},
    {3u, KeyboardOperation::EnableInterrupts}};

void Keyboard::handleInterrupt(KeyboardOperation operation,
                               ProcessorState& processor_state) {
  Word b;

  switch (operation) {
    case KeyboardOperation::Clear:
      LOG(KEYBOARD, info) << "'Clear'";
      state_->clearKey();

      break;

    case KeyboardOperation::Store: {
      LOG(KEYBOARD, info) << "'Store'";

      auto key = state_->key();
      if (!key) {
        processor_state.write(Register::C, 0);
      } else {
        processor_state.write(Register::C, *key);
      }

    } break;

    case KeyboardOperation::Query: {
      LOG(KEYBOARD, info) << "'Query'";

      b = processor_state.read(Register::B);
      auto key = state_->key();
      if (key) {
        processor_state.write(Register::C, *key == b);
      } else {
        processor_state.write(Register::C, 0);
      }

    } break;

    case KeyboardOperation::EnableInterrupts:
      LOG(KEYBOARD, info) << "'EnableInterrupts'";

      b = processor_state.read(Register::B);

      if (b != 0) {
        state_->interrupts_enabled_ = true;
        state_->interrupt_message_ = b;
      } else {
        state_->interrupts_enabled_ = false;
      }

      break;
  }
}

unique_ptr<KeyboardState> Keyboard::start() {
  notify();

  LOG(KEYBOARD, info) << "Started.";

  while (status() == SimulationStatus::Running) {
    auto now = std::chrono::system_clock::now();

    if (state_->key() && state_->interrupts_enabled_ &&
        !state_->was_interrupt_sent_) {
      computer_->interruptQueue().push(state_->interrupt_message_);
      state_->was_interrupt_sent_ = true;
    }

    if (interrupt_sink_.isActive()) {
      LOG(KEYBOARD, info) << "Got interrupt.";

      auto& processor_state = interrupt_sink_.processorState();
      auto a = processor_state.read(Register::A);
      auto operation = get(a, kOperationMap);

      if (operation) {
        handleInterrupt(*operation, processor_state);
      }

      interrupt_sink_.respond();
      LOG(KEYBOARD, info) << "Finished handling interrupt.";
    }

    std::this_thread::sleep_until(now + kKeyboardClockPeriod);
  }

  LOG(KEYBOARD, info) << "Shutting down.";
  return std::move(state_);
}

}  // namespace nebula
