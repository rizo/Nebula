/**
   \file address.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Addressing schemes.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#ifndef NEBULA_ADDRESS_HPP_
#define NEBULA_ADDRESS_HPP_

#pragma once

#include "cycle-cost.hpp"
#include "memory.hpp"
#include "printable.hpp"
#include "processor-state.hpp"

namespace nebula {

namespace error {

class InvalidAddress : public std::runtime_error {
 public:
  explicit InvalidAddress(const std::string& message)
      : std::runtime_error{message} {}
};

}  // namespace error

/**
   \brief All addressing schemes must implement this interface.
 */
class IAddress : public ICycleCost, public IPrintable {
 public:
  /**
     \brief Advance the program counter past the address.
   */
  virtual void advance(ProcessorState&, Memory&) = 0;

  virtual ~IAddress() = default;
};

/**
   \brief Addressing schemes which support storing data.

   Most addresses allow storing to them. An example of a non-storable addresses
   is Pop. That is,
   
   \code{.asm}
   SET    POP, 23
   \endcode

   is undefined.
 */
class IStorable {
 public:
  virtual void store(ProcessorState& state, Memory& memory, Word value) = 0;

  virtual ~IStorable() = default;
};

/**
   \brief Addressing schemes which support loading data.
 */
class ILoadable {
 public:  
  virtual Word load(ProcessorState& state, Memory& memory) = 0;

  virtual ~ILoadable() = default;
};

/**
   \brief Short addresses require no additional words to be encoded.
 */
class ShortAddress : public IAddress {
 public:
  virtual ~ShortAddress() = default;

  void advance(ProcessorState&, Memory&) override {
    // Nothing.
  }

  std::size_t cycleCost() const override { return 0u; }
};

/**
   \brief Long addresses require a whole word of memory to be encoded.

   Unlike a ShortAddress, A LongAddress is encoded in memory such that it
   requires a dedicated word.

   An instruction like

   \code{.asm}
   SET    X, Y
   \endcode

   which has two ShortAddress can fit in a single word. However, an instruction
   like 

   \code{.asm}
   ADD    X, 0xf00d
   \endcode

   requires that the operand 0xf00d be stored in its own word.
 */
class LongAddress : public IAddress {
 protected:
  std::string nextWordAsString() const;

 private:
  optional<Word> next_word_;

 public:
  explicit LongAddress() : next_word_{} {}

  virtual ~LongAddress() = default;

  std::size_t cycleCost() const override { return 1u; }

  void advance(ProcessorState& state, Memory& memory) override;

  /**
     \brief Get the next word of the address.

     This function is idempotent. The first time it is executed, the word
     following the address is fetched as the operand and the PC is
     incremented. Every subsequent call to nextWord() returns the _same_ word
     that was fetched.
   */
  Word nextWord(ProcessorState& state, Memory& memory);
};

/**
   \brief Supported addressing schemes.
 */
namespace address {

/**
   \brief Access registers directly.
 */
class RegisterDirect : public ShortAddress, public ILoadable, public IStorable {
 private:
  Register reg_;

 public:
  explicit RegisterDirect(Register reg) : ShortAddress{}, reg_{reg} {}

  inline Register reg() const {
    return reg_;
  };

  inline Word load(ProcessorState& state, Memory&) override {
    return state.read(reg_);
  }

  inline void store(ProcessorState& state, Memory&, Word value) override {
    state.write(reg_, value);
  }

  inline std::ostream& print(std::ostream& os) const override {
    os << reg_;
    return os;
  }
};

/**
   \brief Memory at the location stored in a register.
 */
class RegisterIndirect : public ShortAddress,
                         public ILoadable,
                         public IStorable {
 private:
  Register reg_;

 public:
  explicit RegisterIndirect(Register reg) : ShortAddress{}, reg_{reg} {}


  inline Register reg() const { return reg_; }

  inline Word load(ProcessorState& state, Memory& memory) override {
    return memory.read(state.read(reg_));
  }

  inline void store(ProcessorState& state,
                    Memory& memory,
                    Word value) override {
    memory.write(state.read(reg_), value);
  }

  inline std::ostream& print(std::ostream& os) const override {
    os << "[" << reg_ << "]";
    return os;
  }
};

/**
   \brief Like RegisterDirect, but with a fixed offset.
 */
class RegisterIndirectOffset : public LongAddress,
                               public ILoadable,
                               public IStorable {
 private:
  Register reg_;

 public:
  explicit RegisterIndirectOffset(Register reg) : LongAddress{}, reg_{reg} {}

  inline Register reg() const { return reg_; }

  Word load(ProcessorState& state, Memory& memory) override;

  void store(ProcessorState& state, Memory& memory, Word value) override;

  inline std::ostream& print(std::ostream& os) const override {
    os << boost::format("[%s + %s]") % reg_ % nextWordAsString();
    return os;
  }
};

/**
   \brief Push a value onto the stack.

   \note This address can _only_ be used as the left-hand operand of an
   instruction.
 */
class Push : public ShortAddress, public IStorable {
 public:
  explicit Push() : ShortAddress{} {}

  void store(ProcessorState& state, Memory& memory, Word value) override;

  inline std::ostream& print(std::ostream& os) const override {
    os << "PUSH";
    return os;
  }
};

/**
   \brief Pop a value from the stack.

   \note This address can _only_ be used as the right-hand operand of an
   instruction.
 */
class Pop : public ShortAddress, public ILoadable {
 public:
  explicit Pop() : ShortAddress{} {}

  Word load(ProcessorState& state, Memory& memory) override;

  inline std::ostream& print(std::ostream& os) const override {
    os << "POP";
    return os;
  }
};

/**
   \brief The top (most recently added thing) of the stack.
 */
class Peek : public ShortAddress, public ILoadable, public IStorable {
 public:
  explicit Peek() : ShortAddress{} {}

  Word load(ProcessorState& state, Memory& memory) override;

  void store(ProcessorState& state, Memory& memory, Word value) override;

  inline std::ostream& print(std::ostream& os) const override {
    os << "[SP]";
    return os;
  }
};

/**
   \brief Explore the stack.
 */
class Pick : public LongAddress, public ILoadable, public IStorable {
 public:
  explicit Pick() : LongAddress{} {}

  Word load(ProcessorState& state, Memory& memory) override;

  void store(ProcessorState& state, Memory& memory, Word value) override;

  inline std::ostream& print(std::ostream& os) const override {
    os << "[SP + " << nextWordAsString() << "]";
    return os;
  }
};

/**
   \brief The address of the top of the stack.
 */
class Sp : public ShortAddress, public ILoadable, public IStorable {
 public:
  explicit Sp() : ShortAddress{} {}

  inline Word load(ProcessorState& state, Memory&) override {
    return state.read(Special::Sp);
  }

  inline void store(ProcessorState& state, Memory&, Word value) override {
    state.write(Special::Sp, value);
  }

  inline std::ostream& print(std::ostream& os) const override {
    os << "SP";
    return os;
  }
};

/**
   \brief The program counter (Special::Pc).
 */
class Pc : public ShortAddress, public ILoadable, public IStorable {
 public:
  explicit Pc() : ShortAddress{} {}

  inline Word load(ProcessorState& state, Memory&) override {
    return state.read(Special::Pc);
  }

  inline void store(ProcessorState& state, Memory&, Word value) override {
    state.write(Special::Pc, value);
  }

  inline std::ostream& print(std::ostream& os) const override {
    os << "PC";
    return os;
  }
};

/**
   \brief The EX Register (Special::Ex).
 */
class Ex : public ShortAddress, public ILoadable, public IStorable {
 public:
  explicit Ex() : ShortAddress{} {}

  inline Word load(ProcessorState& state, Memory&) override {
    return state.read(Special::Ex);
  }

  inline void store(ProcessorState& state, Memory&, Word value) override {
    state.write(Special::Ex, value);
  }

  inline std::ostream& print(std::ostream& os) const override {
    os << "EX";
    return os;
  }
};

/**
   \brief A location in memory.
 */
class Indirect : public LongAddress, public ILoadable, public IStorable {
 public:
  explicit Indirect() : LongAddress{} {}

  Word load(ProcessorState& state, Memory& memory) override;

  void store(ProcessorState&, Memory&, Word) override;

  inline std::ostream& print(std::ostream& os) const override {
    os << "[" << nextWordAsString() << "]";
    return os;
  }
};

/**
   \brief A literal (or constant) value.

   \note Assigning to this address silently does nothing (according to the
   specs...).
 */
class Direct : public LongAddress, public ILoadable, public IStorable {
 public:
  explicit Direct() : LongAddress{} {}

  Word load(ProcessorState& state, Memory& memory) override;

  inline void store(ProcessorState&, Memory&, Word) override {
    // Nothing.
  }

  inline std::ostream& print(std::ostream& os) const override {
    os << nextWordAsString();
    return os;
  }
};

/**
   \brief A special case of Direct with a small encoding.

   This addressing mode is optionally used for small-valued Direct addresses
   that don't need to be encoded as their own word.
 */
class FastDirect : public ShortAddress, public ILoadable, public IStorable {
 private:
  Word value_;

 public:
  explicit FastDirect(Word value) : ShortAddress{}, value_{value} {}

  inline Word value() const { return value_; }

  inline Word load(ProcessorState&, Memory&) override { return value_; }

  inline void store(ProcessorState&, Memory&, Word) override {
    // Nothing.
  }

  inline std::ostream& print(std::ostream& os) const override {
    os << boost::format("0x%04x") % value_;
    return os;
  }
};

}  // namespace address

}  // namespace nebula

#endif  // NEBULA_ADDRESS_HPP_
