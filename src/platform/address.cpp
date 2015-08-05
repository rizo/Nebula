/**
   \file address.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "address.hpp"

namespace nebula {

void LongAddress::advance(ProcessorState& state, Memory& memory) {
  auto pc = state.read(Special::Pc);
  next_word_ = memory.read(pc);

  pc += 1u;
  state.write(Special::Pc, pc);
}

Word LongAddress::nextWord(ProcessorState& state, Memory& memory) {
  if (!next_word_) {
    advance(state, memory);
  }

  return *next_word_;
}

std::string LongAddress::nextWordAsString() const {
  if (!next_word_) {
    return "<next-word>";
  } else {
    return (boost::format("0x%04x") % next_word_.get()).str();
  }
}

namespace address {

Word Direct::load(ProcessorState& state, Memory& memory) {
  return nextWord(state, memory);
}

Word RegisterIndirectOffset::load(ProcessorState& state, Memory& memory) {
  return memory.read(state.read(reg_) + nextWord(state, memory));
}

void RegisterIndirectOffset::store(ProcessorState& state,
                                   Memory& memory,
                                   Word value) {
  memory.write(state.read(reg_) + nextWord(state, memory), value);
}

void Push::store(ProcessorState& state, Memory& memory, Word value) {
  auto sp = state.read(Special::Sp);

  if (sp == 0u) {
    sp = kStackBegin;
  } else {
    sp -= 1u;
  }

  state.write(Special::Sp, sp);
  memory.write(sp, value);
}

Word Pop::load(ProcessorState& state, Memory& memory) {
  auto sp = state.read(Special::Sp);

  if (sp == kStackBegin) {
    state.write(Special::Sp, 0u);
  } else {
    state.write(Special::Sp, sp + 1);
  }

  return memory.read(sp);
}

Word Peek::load(ProcessorState& state, Memory& memory) {
  return memory.read(state.read(Special::Sp));
}

void Peek::store(ProcessorState& state, Memory& memory, Word value) {
  memory.write(state.read(Special::Sp), value);
}

Word Pick::load(ProcessorState& state, Memory& memory) {
  return memory.read(state.read(Special::Sp) + nextWord(state, memory));
}

void Pick::store(ProcessorState& state, Memory& memory, Word value) {
  memory.write(state.read(Special::Sp) + nextWord(state, memory), value);
}

Word Indirect::load(ProcessorState& state, Memory& memory) {
  return memory.read(nextWord(state, memory));
}

void Indirect::store(ProcessorState& state, Memory& memory, Word value) {
  memory.write(nextWord(state, memory), value);
}

}  // namespace address

}  // namespace nebula
