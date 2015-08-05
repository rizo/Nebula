/**
   \file instruction.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Instruction execution.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#ifndef NEBULA_INSTRUCTION_HPP_
#define NEBULA_INSTRUCTION_HPP_

#pragma once

#include "address.hpp"

namespace nebula {

enum class OpCode {
  Set,
  Add,
  Sub,
  Mul,
  Mli,
  Div,
  Dvi,
  Mod,
  Mdi,
  And,
  Bor,
  Xor,
  Shr,
  Asr,
  Shl,
  Ifb,
  Ifc,
  Ife,
  Ifn,
  Ifg,
  Ifa,
  Ifl,
  Ifu,
  Adx,
  Sbx,
  Sti,
  Std
};

enum class SpecialOpCode {
  Jsr,
  Int,
  Iag,
  Ias,
  Rfi,
  Iaq,
  Hwn,
  Hwq,
  Hwi,

  // Extensions
  Abt
};

/**
   \brief All instructions implement this interface.
 */
class IInstruction : public ICycleCost, public IPrintable {
 public:
  virtual ~IInstruction() = default;
  
  /**
     \brief Execute the instruction.

     \note Prior to be executed, the instruction must be advance()-ed.
   */
  virtual void execute(ProcessorState& state, Memory& memory) = 0;

  /**
     \brief Advance the PC to the next instruction following this one.

     \note This must take place prior to execute()-ing this instruction.
   */
  virtual void advance(ProcessorState& state, Memory& memory) = 0;

  /**
     \brief Conditional instructions have special behavior.
   */
  virtual bool isConditional() const = 0;
};

/**
   \brief Types of instructions.
 */
namespace instruction {

/**
   An instruction with a single operand.
 */
class Unary : public IInstruction {
 private:
  SpecialOpCode op_code_;
  unique_ptr<IAddress> address_;

 public:
  explicit Unary(SpecialOpCode op_code, unique_ptr<IAddress> address)
      : op_code_{op_code}, address_{std::move(address)} {}

  inline void advance(ProcessorState& state, Memory& memory) override {
    address_->advance(state, memory);
  }

  inline bool isConditional() const override { return false; }

  void execute(ProcessorState& state, Memory& memory) override;

  std::size_t cycleCost() const override;

  inline SpecialOpCode opCode() const { return op_code_; }

  inline IAddress& address() const { return *address_.get(); }
  /**
     \bug This function should either return a const reference or not be marked
     as const.
   */

  std::ostream& print(std::ostream& os) const override;
};

class Binary : public IInstruction {
 private:
  OpCode op_code_;
  unique_ptr<IAddress> address_b_;
  unique_ptr<IAddress> address_a_;

 public:
  explicit Binary(OpCode op_code,
                  unique_ptr<IAddress> address_b,
                  unique_ptr<IAddress> address_a)
      : op_code_{op_code},
        address_b_{std::move(address_b)},
        address_a_{std::move(address_a)} {}

  inline void advance(ProcessorState& state, Memory& memory) override {
    address_a_->advance(state, memory);
    address_b_->advance(state, memory);
  }

  void execute(ProcessorState& state, Memory& memory) override;

  inline IAddress& addressB() { return *address_b_; }

  inline IAddress& addressA() { return *address_a_; }

  bool isConditional() const override;

  std::size_t cycleCost() const override;

  std::ostream& print(std::ostream& os) const override;
};

}  // namespace instruction

std::ostream& operator<<(std::ostream& os, const OpCode& op_code);

std::ostream& operator<<(std::ostream& os,
                         const SpecialOpCode& special_op_code);

}  // namespace nebula

#endif  // NEBULA_INSTRUCTION_HPP_
