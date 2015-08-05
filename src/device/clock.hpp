/**
   \file clock.hpp
   \copyright 2014 Jesse Haber-Kucharsky
   
   \brief A simple clock.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   \see \p /doc/hw/clock.txt
 */

#ifndef NEBULA_DEVICES_CLOCK_HPP_
#define NEBULA_DEVICES_CLOCK_HPP_

#pragma once

#include <platform/computer.hpp>

DEFINE_LOGGER(CLOCK)

namespace nebula {

/**
   \brief The minimum resolution of the clock.

   1/60th of a second.
 */
const std::chrono::microseconds kClockBasePeriod{16666u};

enum class ClockOperation {
  SetDivider,
  StoreElapsed,
  EnableInterrupts
};

struct ClockState {
  Word divider{1u};
  bool is_on{false};
  bool are_interrupts_enabled{false};
  Word elapsed_ticks{0u};
  Word interrupt_message{0u};
};

class Clock : public Simulation<ClockState>, public IDevice {
 private:
  shared_ptr<Computer> computer_;
  InterruptSink interrupt_sink_;
  ClockState state_{};

  void handleInterrupt(ClockOperation operation,
                       ProcessorState& processor_state);

 public:
  explicit Clock(shared_ptr<Computer> computer)
      : Simulation<ClockState>{},
        computer_{computer},
        interrupt_sink_{computer->registerDevice(deviceInfo())} {}

  inline DeviceInfo deviceInfo() const override {
    return {DeviceID{0x12d0b402}, DeviceManufacturer{0u}, DeviceVersion{1u}};
  }

  unique_ptr<ClockState> start() override;
};

}  // namespace nebula

#endif  // NEBULA_DEVICES_CLOCK_HPP_
