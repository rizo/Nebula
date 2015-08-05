/**
   \file computer.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "computer.hpp"

#include "decode.hpp"

#include <algorithm>
#include <limits>

namespace nebula {

static unique_ptr<IInstruction> fetchNextInstruction(ProcessorState& state,
                                                     Memory& memory) {
  address::Direct address;
  auto word = address.load(state, memory);
  auto instruction = decodeInstruction(word);

  if (!instruction) {
    throw error::MalformedWord(word);
  }

  return instruction;
}

void Computer::executeNextInstruction() {
  LOG(COMPUTER, debug) << *this;

  auto instruction = fetchNextInstruction(*state_, *memory_);
  instruction->advance(*state_, *memory_);

  if (state_->read(Flag::SkipNext) != 0u) {
    state_->tickCycleCount(1u);

    // When conditional instructions are skipped, we continue
    // skipping.
    if (!instruction->isConditional()) {
      state_->write(Flag::SkipNext, false);
    }
  } else {
    // This is a little bit unclear.
    //
    // Most instructions modify and/or read directly from the processor
    // state. However, some of the so-called "special" instructions modify not
    // just the processor state but also the state of the computer itself
    // including information on attached hardware devices and the interrupt
    // queue.
    //
    // Inside of IInstruction::execute(), these special instructions don't
    // actually do anything, and instead perform their functionality in
    // executeSpecialInstruction().
    //
    // Similarly, the special isntructions that *can* execute entirely on the
    // processor state do so inside of IInstruction::execute() and don't do
    // anything inside executeSpecialInstruction().

    instruction->execute(*state_, *memory_);

    if (auto special_instruction =
            dynamic_cast<instruction::Unary*>(instruction.get())) {
      executeSpecialInstruction(*special_instruction);
    }

    state_->tickCycleCount(instruction->cycleCost());
  }
}

void Computer::executeSpecialInstruction(instruction::Unary& instruction) {
  auto op_code = instruction.opCode();

  auto load = [this](IAddress & address)->Word {
    if (auto loadable = dynamic_cast<ILoadable*>(&address)) {
      return loadable->load(*state_, *memory_);
    } else {
      throw error::InvalidAddress{
          "Attempt to load from a non-loadable address."};
    }
  };

  auto store = [this](IAddress& address, Word value) {
    if (auto storable = dynamic_cast<IStorable*>(&address)) {
      storable->store(*state_, *memory_, value);
    } else {
      throw error::InvalidAddress{
          "Attempt to store to a non-storable address."};
    }
  };

  switch (op_code) {
    case SpecialOpCode::Int: {
      interrupt_queue_.push(load(instruction.address()));
    } break;

    case SpecialOpCode::Hwn: {
      store(instruction.address(), devices_.size());
    } break;

    case SpecialOpCode::Hwq: {
      auto& device_info = getDeviceInfo(load(instruction.address()));

      state_->write(Register::A, device_info.id.value & 0xffffu);
      state_->write(Register::B, (device_info.id.value & 0xffff0000u) >> 16u);

      state_->write(Register::X, device_info.manufacturer.value & 0xffffu);
      state_->write(Register::Y,
                    (device_info.manufacturer.value & 0xffff0000u) >> 16u);

      state_->write(Register::C, device_info.version.value);
    } break;

    case SpecialOpCode::Hwi: {
      Word value = load(instruction.address());

      LOG(COMPUTER, info) << format(
                                 "Sending an interrupt to hardware 0x%04x.") %
                                 value;

      auto& interrupt_source = getInterruptSource(value);
      interrupt_source.trigger(std::move(state_));
      state_ = interrupt_source.waitForResponse();

      LOG(COMPUTER, info) << "Resumed control after interrupt.";
    } break;

    case SpecialOpCode::Abt: {
      state_->write(Flag::Aborted, true);

      //
      // Read the error message from memory.
      //

      std::ostringstream message;
      Word message_offset = load(instruction.address());

      if (message_offset != 0) {
        Word message_length = memory_->read(message_offset);

        for (Word character_index = 0u; character_index < message_length;
             ++character_index) {
          auto ch = static_cast<char>(
              memory_->read(message_offset + 1u + character_index));
          message << ch;
        }
      }

      state_->setError(state_->read(Special::Pc), message.str());
    } break;

    case SpecialOpCode::Jsr:
    case SpecialOpCode::Iag:
    case SpecialOpCode::Ias:
    case SpecialOpCode::Rfi:
    case SpecialOpCode::Iaq:
      // Handled by instruction.cpp.
      break;
  }
}

InterruptSink Computer::registerDevice(DeviceInfo device_info) {
  if (devices_.size() >= std::numeric_limits<Word>::max()) {
    throw error::TooManyDevices{};
  }

  devices_.emplace_back(device_info);
  auto interrupt = std::make_shared<Interrupt>();
  interrupt_sources_.emplace_back(interrupt);
  return InterruptSink(interrupt);
}

std::vector<std::tuple<Word, std::unique_ptr<IInstruction>>>
Computer::lookAhead(size_t num_instructions) {
  std::vector<std::tuple<Word, std::unique_ptr<IInstruction>>> result;
  Word old_pc = state_->read(Special::Pc);
  Word current_pc = old_pc;

  for (std::size_t i = 0; i < num_instructions; ++i) {
    try {
      auto instruction = fetchNextInstruction(*state_, *memory_);
      instruction->advance(*state_, *memory_);
      result.emplace_back(std::make_tuple(current_pc, std::move(instruction)));
      current_pc = state_->read(Special::Pc);
    }
    catch (error::MalformedWord&) {
      result.emplace_back(std::make_tuple(current_pc, nullptr));
      break;
    }
  }

  state_->write(Special::Pc, old_pc);

  return result;
}

void Computer::push(Word value) {
  address::Push address;
  address.store(*state_, *memory_, value);
}

void Computer::step() {
  auto interrupt_message = interrupt_queue_.pop();

  if (interrupt_message && (state_->read(Special::Ia) != 0u)) {
    push(state_->read(Special::Pc));
    push(state_->read(Register::A));
    state_->write(Special::Pc, state_->read(Special::Ia));
    state_->write(Register::A, *interrupt_message);

    LOG(COMPUTER, info)
        << format("Inside interrupt handler @ 0x%04x with message 0x%04x") %
               state_->read(Special::Pc) % *interrupt_message;
  }

  executeNextInstruction();
}

std::ostream& Computer::print(std::ostream& os) const {
  // Dump the processor state.
  os << *state_ << "\n";

  // Dump the contents of the stack (including the number of elements).
  Word sp = state_->read(Special::Sp);
  Word num_stack_elements = kStackBegin - sp + 1;
  os << "STACK (";

  if (num_stack_elements == 0u) {
    os << "Empty";
  } else {
    os << num_stack_elements;
  }

  os << ")\n";

  const Word kMaxDisplayedStackElements{10u};

  Word num_displayed_elements{
      std::min(kMaxDisplayedStackElements, num_stack_elements)};

  if (num_displayed_elements > 0u) {
    os << "[";

    for (Word element_index = 0u; element_index < num_displayed_elements;
         ++element_index) {
      Word address = sp + element_index;
      Word element = memory_->read(address);
      os << format("0x%04x") % element;

      if (element_index != (num_displayed_elements - 1u)) {
        os << ", ";
      }
    }

    if (num_displayed_elements < num_stack_elements) {
      os << " ...";
    }

    os << "]" << std::endl;
  }

  return os;
}

}  // namespace nebula
