/**
   \file instruction.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "instruction.hpp"

#include <functional>

namespace nebula {

static std::size_t cycleCost(SpecialOpCode op_code) {
  switch (op_code) {
    case SpecialOpCode::Jsr:
      return 3u;

    case SpecialOpCode::Int:
      return 4u;

    case SpecialOpCode::Iag:
      return 1u;

    case SpecialOpCode::Ias:
      return 1u;

    case SpecialOpCode::Rfi:
      return 3u;

    case SpecialOpCode::Iaq:
      return 2u;

    case SpecialOpCode::Hwn:
      return 2u;

    case SpecialOpCode::Hwq:
      return 4u;

    case SpecialOpCode::Hwi:
      return 4u;

    case SpecialOpCode::Abt:
      return 4u;
  }

  return 0u;
}

static std::size_t cycleCost(OpCode op_code) {
  switch (op_code) {
    case OpCode::Set:
      return 1u;

    case OpCode::Add:
      return 2u;

    case OpCode::Sub:
      return 2u;

    case OpCode::Mul:
      return 2u;

    case OpCode::Mli:
      return 2u;

    case OpCode::Div:
      return 3u;

    case OpCode::Dvi:
      return 3u;

    case OpCode::Mod:
      return 3u;

    case OpCode::Mdi:
      return 3u;

    case OpCode::And:
      return 1u;

    case OpCode::Bor:
      return 1u;

    case OpCode::Xor:
      return 1u;

    case OpCode::Shr:
      return 1u;

    case OpCode::Asr:
      return 1u;

    case OpCode::Shl:
      return 1u;

    case OpCode::Ifb:
    case OpCode::Ifc:
    case OpCode::Ife:
    case OpCode::Ifn:
    case OpCode::Ifg:
    case OpCode::Ifa:
    case OpCode::Ifl:
    case OpCode::Ifu:
      return 2u;

    case OpCode::Adx:
      return 3u;

    case OpCode::Sbx:
      return 3u;

    case OpCode::Sti:
      return 2u;

    case OpCode::Std:
      return 2u;
  }

  // It's necessary to return explicitly here even though all the
  // cases are handled in the previous 'switch' construct in order to
  // avoid the compiler warning.
  return 0u;
}

namespace instruction {

bool Binary::isConditional() const {
  switch (op_code_) {
    case OpCode::Ifb:
    case OpCode::Ifc:
    case OpCode::Ife:
    case OpCode::Ifn:
    case OpCode::Ifg:
    case OpCode::Ifa:
    case OpCode::Ifl:
    case OpCode::Ifu:
      return true;

    default:
      return false;
  }
}

void Binary::execute(ProcessorState& state, Memory& memory) {
  Word x, y;
  DoubleWord xd, yd, zd;
  SignedDoubleWord xdi, ydi, zdi;

  auto store = [&state, &memory](const unique_ptr<IAddress>& address,
                                 Word value) {
    if (auto storable = dynamic_cast<IStorable*>(address.get())) {
      storable->store(state, memory, value);
    } else {
      throw error::InvalidAddress{
          "Attempt to store to a non-storable address."};
    }
  };

  auto load = [&state, &memory ](const unique_ptr<IAddress> & address)->Word {
    if (auto loadable = dynamic_cast<ILoadable*>(address.get())) {
      return loadable->load(state, memory);
    } else {
      throw error::InvalidAddress{
          "Attempt to load from a non-loadable address."};
    }
  };

  auto apply = [&](const std::function<Word(Word, Word)>& f) {
    Word y = load(address_a_);
    Word x = load(address_b_);
    store(address_b_, f(x, y));
  };

  auto applyToDouble = [&](
      const std::function<DoubleWord(DoubleWord, DoubleWord)>& f,
      const std::function<void(DoubleWord)>& action) {
    DoubleWord yd{load(address_a_)};
    DoubleWord xd{load(address_b_)};

    DoubleWord zd = f(xd, yd);
    store(address_b_, static_cast<Word>(zd));
    action(zd);
  };

  auto applyToSigned = [&](
      const std::function<SignedWord(SignedWord, SignedWord)>& f) {
    auto yi = static_cast<SignedWord>(load(address_a_));
    auto xi = static_cast<SignedWord>(load(address_b_));

    auto zi = f(xi, yi);
    store(address_b_, static_cast<Word>(zi));
  };

  auto skipUnless = [&](const std::function<bool(Word, Word)>& test) {
    Word y{load(address_a_)};
    Word x{load(address_b_)};

    state.write(Flag::SkipNext, !test(x, y));
  };

  auto signedSkipUnless = [&](
      const std::function<bool(SignedWord, SignedWord)>& test) {
    auto yi = static_cast<SignedWord>(load(address_a_));
    auto xi = static_cast<SignedWord>(load(address_b_));

    state.write(Flag::SkipNext, !test(xi, yi));
  };

  switch (op_code_) {
    case OpCode::Set:
      store(address_b_, load(address_a_));
      break;

    case OpCode::Add:
      applyToDouble([](DoubleWord x, DoubleWord y) { return x + y; },
                    [&state](DoubleWord z) {
        if (z > 0xffffu) {
          state.write(Special::Ex, 1u);
        } else {
          state.write(Special::Ex, 0u);
        }
      });
      break;

    case OpCode::Sub:
      applyToDouble([](DoubleWord x, DoubleWord y) { return x - y; },
                    [&state](DoubleWord z) {
        if (z > 0xffffu) {
          state.write(Special::Ex, 0xffffu);
        } else {
          state.write(Special::Ex, 0u);
        }
      });
      break;

    case OpCode::Mul:
      applyToDouble([](DoubleWord x, DoubleWord y) { return x * y; },
                    [&state](DoubleWord z) {
        state.write(Special::Ex, static_cast<Word>((z >> 16u) & 0xffffu));
      });
      break;

    case OpCode::Mli:
      applyToSigned([](SignedWord x, SignedWord y) { return x * y; });
      break;

    case OpCode::Div:
      y = load(address_a_);
      x = load(address_b_);

      if (y == 0u) {
        store(address_b_, 0u);
        state.write(Special::Ex, 0u);
      } else {
        store(address_b_, static_cast<Word>(x / y));
        state.write(Special::Ex, static_cast<Word>(((x << 16u) / y) & 0xffffu));
      }
      break;

    case OpCode::Dvi:
      applyToSigned([](SignedWord x, SignedWord y) { return x / y; });
      break;

    case OpCode::Mod:
      apply([](Word x, Word y)->Word {
        if (y == 0u) {
          return 0u;
        } else {
          return x % y;
        }
      });
      break;

    case OpCode::Mdi:
      applyToSigned([](SignedWord xi, SignedWord yi)->SignedWord {
        if (yi == 0) {
          return 0u;
        } else {
          return xi % yi;
        }
      });
      break;

    case OpCode::And:
      apply([](Word x, Word y) { return x & y; });
      break;

    case OpCode::Bor:
      apply([](Word x, Word y) { return x | y; });
      break;

    case OpCode::Xor:
      apply([](Word x, Word y) { return x ^ y; });
      break;

    case OpCode::Shr:
      yd = load(address_a_);
      xd = load(address_b_);

      zd = xd >> yd;
      state.write(Special::Ex,
                  static_cast<Word>(((xd << 16u) >> yd) & 0xffffu));
      store(address_b_, static_cast<Word>(zd));
      break;

    case OpCode::Asr:
      ydi = static_cast<SignedDoubleWord>(load(address_a_));
      xdi = static_cast<SignedDoubleWord>(load(address_b_));

      zdi = xdi >> ydi;
      state.write(Special::Ex,
                  static_cast<Word>(((xdi << 16) >> ydi) & 0xffff));
      store(address_b_, static_cast<Word>(zdi));
      break;

    case OpCode::Shl:
      yd = load(address_a_);
      xd = load(address_b_);

      zd = xd << yd;
      store(address_b_, static_cast<Word>((xd << yd) & 0xffffu));
      break;

    case OpCode::Ifb:
      skipUnless([](Word x, Word y) { return (x & y) != 0u; });
      break;

    case OpCode::Ifc:
      skipUnless([](Word x, Word y) { return (x & y) == 0; });
      break;

    case OpCode::Ife:
      skipUnless([](Word x, Word y) { return x == y; });
      break;

    case OpCode::Ifn:
      skipUnless([](Word x, Word y) { return x != y; });
      break;

    case OpCode::Ifg:
      skipUnless([](Word x, Word y) { return x > y; });
      break;

    case OpCode::Ifa:
      signedSkipUnless([](SignedWord x, SignedWord y) { return x > y; });
      break;

    case OpCode::Ifl:
      skipUnless([](Word x, Word y) { return x < y; });
      break;

    case OpCode::Ifu:
      signedSkipUnless([](SignedWord x, SignedWord y) { return x < y; });
      break;

    case OpCode::Adx:
      yd = load(address_a_);
      xd = load(address_b_);

      zd = xd + yd + state.read(Special::Ex);

      if (zd > 0xffffu) {
        state.write(Special::Ex, 1u);
      } else {
        state.write(Special::Ex, 0u);
      }

      store(address_b_, static_cast<Word>(zd));
      break;

    case OpCode::Sbx:
      yd = load(address_a_);
      xd = load(address_b_);

      zd = xd - yd + state.read(Special::Ex);

      if (zd > 0xffffu) {
        state.write(Special::Ex, 1u);
      } else {
        state.write(Special::Ex, 0u);
      }

      store(address_b_, static_cast<Word>(zd));
      break;

    case OpCode::Sti:
      y = load(address_a_);

      store(address_b_, y);
      state.write(Register::I, state.read(Register::I) + 1u);
      state.write(Register::J, state.read(Register::J) + 1u);
      break;

    case OpCode::Std:
      y = load(address_a_);

      store(address_b_, y);
      state.write(Register::I, state.read(Register::I) - 1u);
      state.write(Register::J, state.read(Register::J) - 1u);
      break;
  }
}

std::size_t Binary::cycleCost() const {
  return nebula::cycleCost(op_code_) + address_b_->cycleCost() +
         address_a_->cycleCost();
}

std::ostream& Binary::print(std::ostream& os) const {
  os << op_code_ << "\t" << *address_b_ << ", " << *address_a_;
  return os;
}

std::size_t Unary::cycleCost() const {
  return nebula::cycleCost(op_code_) + address_->cycleCost();
}

void Unary::execute(ProcessorState& state, Memory& memory) {
  auto push = [&](Word value) {
    address::Push address;
    address.store(state, memory, value);
  };

  auto pop = [&] {
    address::Pop address;
    return address.load(state, memory);
  };

  auto store = [&state, &memory, this](Word value) {
    if (auto storable = dynamic_cast<IStorable*>(address_.get())) {
      storable->store(state, memory, value);
    } else {
      throw error::InvalidAddress{
          "Attempt to store to a non-storable address."};
    }
  };

  auto load = [&state, &memory, this ]()->Word {
    if (auto loadable = dynamic_cast<ILoadable*>(address_.get())) {
      return loadable->load(state, memory);
    } else {
      throw error::InvalidAddress{
          "Attempt to load from a non-loadable address."};
    }
  };

  switch (op_code_) {
    case SpecialOpCode::Jsr:
      push(state.read(Special::Pc));
      state.write(Special::Pc, load());
      break;

    case SpecialOpCode::Int:
    case SpecialOpCode::Hwn:
    case SpecialOpCode::Hwq:
    case SpecialOpCode::Hwi:
    case SpecialOpCode::Abt:
      // Handled by computer.cpp.
      break;

    case SpecialOpCode::Iag:
      store(state.read(Special::Ia));
      break;

    case SpecialOpCode::Ias:
      state.write(Special::Ia, load());
      break;

    case SpecialOpCode::Rfi:
      state.write(Flag::OnlyQueueInterrupts, false);
      state.write(Register::A, pop());
      state.write(Special::Pc, pop());
      break;

    case SpecialOpCode::Iaq:
      state.write(Flag::OnlyQueueInterrupts, load() != 0u);
      break;
  }
}

std::ostream& Unary::print(std::ostream& os) const {
  os << op_code_ << "\t" << *address_;
  return os;
}

}  // namespace instruction

std::ostream& operator<<(std::ostream& os, const OpCode& op_code) {
  switch (op_code) {
    case OpCode::Set:
      os << "SET";
      break;

    case OpCode::Add:
      os << "ADD";
      break;

    case OpCode::Sub:
      os << "SUB";
      break;

    case OpCode::Mul:
      os << "MUL";
      break;

    case OpCode::Mli:
      os << "MLI";
      break;

    case OpCode::Div:
      os << "DIV";
      break;

    case OpCode::Dvi:
      os << "DVI";
      break;

    case OpCode::Mod:
      os << "MOD";
      break;

    case OpCode::Mdi:
      os << "MDI";
      break;

    case OpCode::And:
      os << "AND";
      break;

    case OpCode::Bor:
      os << "BOR";
      break;

    case OpCode::Xor:
      os << "XOR";
      break;

    case OpCode::Shr:
      os << "SHR";
      break;

    case OpCode::Asr:
      os << "ASR";
      break;

    case OpCode::Shl:
      os << "SHL";
      break;

    case OpCode::Ifb:
      os << "IFB";
      break;

    case OpCode::Ifc:
      os << "IFC";
      break;

    case OpCode::Ife:
      os << "IFE";
      break;

    case OpCode::Ifn:
      os << "IFN";
      break;

    case OpCode::Ifg:
      os << "IFG";
      break;

    case OpCode::Ifa:
      os << "IFA";
      break;

    case OpCode::Ifl:
      os << "IFL";
      break;

    case OpCode::Ifu:
      os << "IFU";
      break;

    case OpCode::Adx:
      os << "ADX";
      break;

    case OpCode::Sbx:
      os << "SBX";
      break;

    case OpCode::Sti:
      os << "STI";
      break;

    case OpCode::Std:
      os << "STD";
      break;
  }

  return os;
}

std::ostream& operator<<(std::ostream& os,
                         const SpecialOpCode& special_op_code) {
  switch (special_op_code) {
    case SpecialOpCode::Jsr:
      os << "JSR";
      break;

    case SpecialOpCode::Int:
      os << "INT";
      break;

    case SpecialOpCode::Iag:
      os << "IAG";
      break;

    case SpecialOpCode::Ias:
      os << "IAS";
      break;

    case SpecialOpCode::Rfi:
      os << "RFI";
      break;

    case SpecialOpCode::Iaq:
      os << "IAQ";
      break;

    case SpecialOpCode::Hwn:
      os << "HWN";
      break;

    case SpecialOpCode::Hwq:
      os << "HWQ";
      break;

    case SpecialOpCode::Hwi:
      os << "HWI";
      break;

    case SpecialOpCode::Abt:
      os << "ABT";
      break;
  }

  return os;
}

}  // namespace nebula
