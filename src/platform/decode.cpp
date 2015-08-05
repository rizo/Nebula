/**
   \file decode.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "decode.hpp"

#include <array>

namespace nebula {

const static std::array<Register, 8> kRegisters{
    {Register::A, Register::B, Register::C, Register::X,
     Register::Y, Register::Z, Register::I, Register::J}};

static optional<Register> decodeRegister(Word code) {
  if (code >= kRegisters.size()) {
    return {};
  }

  return kRegisters[static_cast<std::size_t>(code)];
}

optional<OpCode> decodeOpCode(Word code) {
  switch (code) {
    case 0x01u:
      return OpCode::Set;
    case 0x02u:
      return OpCode::Add;
    case 0x03u:
      return OpCode::Sub;
    case 0x04u:
      return OpCode::Mul;
    case 0x05u:
      return OpCode::Mli;
    case 0x06u:
      return OpCode::Div;
    case 0x07u:
      return OpCode::Dvi;
    case 0x08u:
      return OpCode::Mod;
    case 0x09u:
      return OpCode::Mdi;
    case 0x0au:
      return OpCode::And;
    case 0x0bu:
      return OpCode::Bor;
    case 0x0cu:
      return OpCode::Xor;
    case 0x0du:
      return OpCode::Shr;
    case 0x0eu:
      return OpCode::Asr;
    case 0x0fu:
      return OpCode::Shl;
    case 0x10u:
      return OpCode::Ifb;
    case 0x11u:
      return OpCode::Ifc;
    case 0x12u:
      return OpCode::Ife;
    case 0x13u:
      return OpCode::Ifn;
    case 0x14u:
      return OpCode::Ifg;
    case 0x15u:
      return OpCode::Ifa;
    case 0x16u:
      return OpCode::Ifl;
    case 0x17u:
      return OpCode::Ifu;
    case 0x1au:
      return OpCode::Adx;
    case 0x1bu:
      return OpCode::Sbx;
    case 0x1eu:
      return OpCode::Sti;
    case 0x1fu:
      return OpCode::Std;
  }

  return {};
}

optional<SpecialOpCode> decodeSpecialOpCode(Word code) {
  switch (code) {
    case 0x01u:
      return SpecialOpCode::Jsr;
    case 0x08u:
      return SpecialOpCode::Int;
    case 0x09u:
      return SpecialOpCode::Iag;
    case 0x0au:
      return SpecialOpCode::Ias;
    case 0x0bu:
      return SpecialOpCode::Rfi;
    case 0x0cu:
      return SpecialOpCode::Iaq;
    case 0x10u:
      return SpecialOpCode::Hwn;
    case 0x11u:
      return SpecialOpCode::Hwq;
    case 0x12u:
      return SpecialOpCode::Hwi;
    case 0x15u:
      return SpecialOpCode::Abt;
  }

  return {};
}

unique_ptr<IAddress> decodeAddress(AddressContext context, Word code) {
  if (code <= 0x07u) {
    return make_unique<address::RegisterDirect>(*decodeRegister(code));
  } else if ((code >= 0x08u) && (code <= 0x0fu)) {
    return make_unique<address::RegisterIndirect>(
        *decodeRegister(code - 0x08u));
  } else if ((code >= 0x10u) && (code <= 0x17u)) {
    return make_unique<address::RegisterIndirectOffset>(
        *decodeRegister(code - 0x10u));
  } else if ((context == AddressContext::B) && (code == 0x18u)) {
    return make_unique<address::Push>();
  } else if ((context == AddressContext::A) && (code == 0x18u)) {
    return make_unique<address::Pop>();
  } else if (code == 0x19u) {
    return make_unique<address::Peek>();
  } else if (code == 0x1au) {
    return make_unique<address::Pick>();
  } else if (code == 0x1bu) {
    return make_unique<address::Sp>();
  } else if (code == 0x1cu) {
    return make_unique<address::Pc>();
  } else if (code == 0x1du) {
    return make_unique<address::Ex>();
  } else if (code == 0x1eu) {
    return make_unique<address::Indirect>();
  } else if (code == 0x1fu) {
    return make_unique<address::Direct>();
  } else if ((context == AddressContext::A) && (code >= 0x20u) &&
             (code <= 0x3fu)) {
    return make_unique<address::FastDirect>(code - 0x21u);
  } else {
    return nullptr;
  }
}

static unique_ptr<IInstruction> decodeBinaryInstruction(Word word) {
  // Is there a nicer way to do this?

  auto op_code = decodeOpCode(word & 0x1fu);
  if (!op_code) return nullptr;

  auto address_a = decodeAddress(AddressContext::A, (word & 0xfc00u) >> 10u);
  if (!address_a) return nullptr;

  auto address_b = decodeAddress(AddressContext::B, (word & 0x3e0) >> 5u);
  if (!address_b) return nullptr;

  return make_unique<instruction::Binary>(
      *op_code, std::move(address_b), std::move(address_a));
}

static unique_ptr<IInstruction> decodeUnaryInstruction(Word word) {
  auto op_code = decodeSpecialOpCode((word & 0x3e0) >> 5u);
  if (!op_code) return nullptr;

  auto address = decodeAddress(AddressContext::A, (word & 0xfc00) >> 10u);
  if (!address) return nullptr;

  return make_unique<instruction::Unary>(*op_code, std::move(address));
}

unique_ptr<IInstruction> decodeInstruction(Word word) {
  auto binary_instruction = decodeBinaryInstruction(word);
  if (binary_instruction) {
    return binary_instruction;
  } else {
    return decodeUnaryInstruction(word);
  }
}

}  // namespace nebula
