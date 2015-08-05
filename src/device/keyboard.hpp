/**
   \file keyboard.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief An emulated keyboard supporting the ASCII character set.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   \see \p /doc/hw/keyboard.txt
 */

#ifndef NEBULA_DEVICE_KEYBOARD_HPP_
#define NEBULA_DEVICE_KEYBOARD_HPP_

#pragma once

#include <platform/computer.hpp>

#include <mutex>

DEFINE_LOGGER(KEYBOARD)

namespace nebula {

/**
   \brief The internal clock period of the keyboard.

   This is effectively the rate at which the keyboard can detect new key
   presses.
 */
const std::chrono::milliseconds kKeyboardClockPeriod{5};

enum class KeyboardOperation {
  Clear,
  Store,
  Query,
  EnableInterrupts
};

/**
   \brief Internal keyboard state.

   This class is structured a little differently than other device
   implementations. The keyboard state is shared betweenthe keyboard
   implementation and the main I/O thread (which is the source of keys). Thus,
   instead of a simple struct, we have a class with a mutex to ensure exclusive
   access to the key buffer.
 */
class KeyboardState {
 private:
  optional<Word> key_{};
  bool was_interrupt_sent_{false};
  bool interrupts_enabled_{false};
  Word interrupt_message_{0u};
  std::mutex mutex_{};

 public:
  inline optional<Word> key() {
    std::lock_guard<std::mutex> lock{mutex_};
    return key_;
  }

  void setKey(Word key);

  inline void clearKey() {
    std::lock_guard<std::mutex> lock{mutex_};
    key_ = boost::none;
  }

  friend class Keyboard;
};

class Keyboard : public Simulation<KeyboardState>, public IDevice {
 private:
  shared_ptr<Computer> computer_;
  InterruptSink interrupt_sink_;
  unique_ptr<KeyboardState> state_;

  void handleInterrupt(KeyboardOperation operation,
                       ProcessorState& processor_state);

 public:
  explicit Keyboard(shared_ptr<Computer> computer)
      : Simulation<KeyboardState>{},
        computer_{computer},
        interrupt_sink_{computer->registerDevice(deviceInfo())},
        state_{make_unique<KeyboardState>()} {}

  inline KeyboardState& state() { return *state_; }

  inline DeviceInfo deviceInfo() const override {
    return {DeviceID{0x30cf7406}, DeviceManufacturer{0}, DeviceVersion{1}};
  }

  unique_ptr<KeyboardState> start() override;
};

}  // namespace nebula

#endif  // NEBULA_DEVICE_KEYBOARD_HPP_
