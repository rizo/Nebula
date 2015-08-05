/**
   \file computer.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Tie together the processor and the hardware.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#ifndef NEBULA_COMPUTER_HPP_
#define NEBULA_COMPUTER_HPP_

#pragma once

#include "instruction.hpp"
#include "interrupt.hpp"
#include "processor-state.hpp"
#include "memory.hpp"

#include <chrono>

DEFINE_LOGGER(COMPUTER)

namespace nebula {

namespace error {

class InvalidDeviceIndex : std::out_of_range {
 public:
  explicit InvalidDeviceIndex(Word index)
      : std::out_of_range{
            (format("Invalid device index: 0x%04x.") % index).str()} {}
};

class TooManyDevices : std::out_of_range {
 public:
  explicit TooManyDevices()
      : std::out_of_range{"Maximum number of devices added."} {}
};

}  // namespace error

/**
   \brief When more than this many interrupts are queued, fires start.
 */
const std::size_t kMaxQueuedInterrupts = 256u;

struct DeviceID {
  DoubleWord value;
  explicit DeviceID(DoubleWord value) : value{value} {}
};

struct DeviceManufacturer {
  DoubleWord value;
  explicit DeviceManufacturer(DoubleWord value) : value{value} {}
};

struct DeviceVersion {
  Word value;
  explicit DeviceVersion(Word value) : value{value} {}
};

struct DeviceInfo {
  DeviceID id;
  DeviceManufacturer manufacturer;
  DeviceVersion version;
};

class IDevice {
 public:
  virtual DeviceInfo deviceInfo() const = 0;
};

/**
   \brief Manage the processor and the hardware devices.

   This is the motherboard, of sorts.

   Hardware devices register themselves with the computer in order to receive
   interrupts from the processor and also to allow themselves to be referenced
   from inside programs via their assigned index.

   Also stored is an internal InterruptQueue, which hardware devices interract
   with directly to queue hardware-generated interrupts for the processor.
 */
class Computer : public IPrintable {
 private:
  unique_ptr<ProcessorState> state_;
  shared_ptr<Memory> memory_;
  InterruptQueue interrupt_queue_;
  std::vector<InterruptSource> interrupt_sources_;
  std::vector<DeviceInfo> devices_;

  void executeSpecialInstruction(instruction::Unary& instruction);

  inline InterruptSource& getInterruptSource(Word index) {
    if (index >= interrupt_sources_.size()) {
      throw error::InvalidDeviceIndex{index};
    }

    return interrupt_sources_[index];
  }

  inline DeviceInfo& getDeviceInfo(Word index) {
    if (index >= devices_.size()) {
      throw error::InvalidDeviceIndex{index};
    }

    return devices_[index];
  }

  /**
     \brief A small helper for pushing onto the stack.
   */
  void push(Word value);

  void executeNextInstruction();

 public:
  explicit Computer(unique_ptr<ProcessorState> state, shared_ptr<Memory> memory)
      : state_{std::move(state)},
        memory_{memory},
        interrupt_queue_{kMaxQueuedInterrupts},
        interrupt_sources_{},
        devices_{} {}

  inline const ProcessorState& state() const { return *state_; }

  inline InterruptQueue& interruptQueue() { return interrupt_queue_; }

  /**
     \brief Retrieve the next \p num_instructions from memory.
   */
  std::vector<std::tuple<Word, std::unique_ptr<IInstruction>>> lookAhead(
      std::size_t num_instructions);

  /**
     \brief Load and execute the next instruction and handle any interrupts.
   */
  void step();

  InterruptSink registerDevice(DeviceInfo device_info);

  std::ostream& print(std::ostream& os) const override;

  // This is to allow the execution manager for the computer direct access to
  // the processor state owned by the computer.
  friend class ExecutionManager;
};

}  // namespace nebula

#endif  // NEBULA_COMPUTER_HPP_
