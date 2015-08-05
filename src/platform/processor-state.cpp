/**
   \file processor-state.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "processor-state.hpp"

namespace nebula {

Word ProcessorState::read(Register reg) const {
  return registers_[static_cast<std::size_t>(reg)];
}

Word ProcessorState::read(Special sp) const {
  return specials_[static_cast<std::size_t>(sp)];
}

Word ProcessorState::read(Flag fg) const {
  return static_cast<Word>(flags_.test(static_cast<std::uint8_t>(fg)));
}

void ProcessorState::write(Register reg, Word value) {
  registers_[static_cast<std::size_t>(reg)] = value;
}

void ProcessorState::write(Special sp, Word value) {
  specials_[static_cast<std::size_t>(sp)] = value;
}

void ProcessorState::write(Flag fg, bool value) {
  flags_.set(static_cast<std::uint8_t>(fg), value);
}

void ProcessorState::setError(Word pc, std::string message) {
  ErrorInformation error_info;
  error_info.message = std::move(message);  
  error_info.pc = pc;
  error_info_ = std::move(error_info);
}

std::ostream& operator<<(std::ostream& os, const Register& reg) {
  // GCC is stupid, and complains that `value' could be uninitialized because it
  // can't tell that all possible values of `Register' are accounted for in the
  // enumeration. Give it a dummy value.
  char value = ' ';

  switch (reg) {
    case Register::A:
      value = 'A';
      break;

    case Register::B:
      value = 'B';
      break;

    case Register::C:
      value = 'C';
      break;

    case Register::X:
      value = 'X';
      break;

    case Register::Y:
      value = 'Y';
      break;

    case Register::Z:
      value = 'Z';
      break;

    case Register::I:
      value = 'I';
      break;

    case Register::J:
      value = 'J';
      break;
  }

  os << value;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Special& special) {
  std::string value;

  switch (special) {
    case Special::Pc:
      value = "PC";
      break;

    case Special::Sp:
      value = "SP";
      break;

    case Special::Ex:
      value = "EX";
      break;

    case Special::Ia:
      value = "IA";
      break;
  }

  os << value;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Flag& flag) {
  std::string value;

  switch (flag) {
    case Flag::SkipNext:
      value = "SkipNext";
      break;

    case Flag::OnlyQueueInterrupts:
      value = "OnlyQueueInterrupts";
      break;

    case Flag::Aborted:
      value = "Aborted";
      break;
  }

  os << value;
  return os;
}

std::ostream& ProcessorState::print(std::ostream& os) const {
  os << "REGISTERS\n";
  os << format("A: 0x%04x    X: 0x%04x\n") % read(Register::A) %
            read(Register::X);
  os << format("B: 0x%04x    Y: 0x%04x\n") % read(Register::B) %
            read(Register::Y);
  os << format("C: 0x%04x    Z: 0x%04x\n") % read(Register::C) %
            read(Register::Z);
  os << format("I: 0x%04x    J: 0x%04x\n") % read(Register::I) %
            read(Register::J);

  os << "\nSPECIALS\n";
  os << format("PC: 0x%04x   SP: 0x%04x\n") % read(Special::Pc) %
            read(Special::Sp);
  os << format("IA: 0x%04x   EX: 0x%04x\n") % read(Special::Ia) %
            read(Special::Ex);

  os << "\nFLAGS\n"
     << format("SkipNext           : %s\n") % read(Flag::SkipNext)
     << format("OnlyQueueInterrupts: %s\n") % read(Flag::OnlyQueueInterrupts)
     << format("Aborted            : %s\n") % read(Flag::Aborted);

  return os;
}

}  // namespace nebula
