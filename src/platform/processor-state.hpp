/**
   \file processor-state.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief DCPU16 processor.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   The result of executing instructions on the DCPU16 processor is dictated
   exlusively through it's internal state, which include registers, a stack, and
   some special execution flags.
 */

#ifndef NEBULA_PROCESSOR_STATE_HPP_
#define NEBULA_PROCESSOR_STATE_HPP_

#pragma once

#include "prelude.hpp"
#include "printable.hpp"

#include <bitset>
#include <ostream>
#include <vector>

DEFINE_LOGGER(PROCESSOR)

namespace nebula {

/**
   \brief General-purpose registers.

   A register can hold any value that fits in a ::Word.
 */
enum class Register {
  A,
  B,
  C,
  X,
  Y,
  Z,
  I,
  J
};

/**
   \brief Special registers.

   Special registers are also ::Word sized, but dictate special functionality in
   the processor.
 */
enum class Special {
  /**
     \brief Program counter.

     The program counter holds the address into memory of the next instruction
     that will be executed by the DCPU16.
   */
  Pc,

  /**
     \brief Stack pointer.

     The stack pointer holds the address of the next-available location in the
     stack. The stack grows *downwards*, so the top of the stack is 0xffff and
     the last entry is located at 0x0.
   */
  Sp,

  /**
     \brief Execution value.

     The EX register is used in conjunction with arithemtic and logical
     instructions to complement their functionality. For instance, if an
     addition operation overflows, then EX is set to 1.
   */
  Ex,

  /**
     \brief Interrupt handler address.

     The DCPU16 allows for a single interrupt handler, where the source of
     interrupts can be deduced via the value of the interrupt code. The IA
     register holds the address of this handler.
   */
  Ia
};

/**
   \brief Processor flags.

   These binary flags dictate the behavior of the DCPU16 while its executing
   instructions.
 */
enum class Flag : std::uint8_t {
  /**
     \brief Skip the next instruction.

     If this flag is enabled, then the next instruction (indicated by the value
     of Register::Pc) will not be executed, but it will be loaded and the state
     will be modified as if it *were* executed (with the exception of the
     effects of executing the instruction itself).
   */
  SkipNext,

  /**
     \brief Don't handle interrupts.

     When this flag is enabled, the DCPU16 will queue interrupts, but not handle
     them via the interrupt handler.
   */
  OnlyQueueInterrupts,

  /**
     \brief The DCPU16 has aborted.

     When the DCPU16 is in the aborted state, it doesn't execute any
     instructions nor handle interrupts.
   */
  Aborted,
};

/**
   The location of the first element of the processor stack.
 */
const Word kStackBegin = 0xffffu;

/**
   \brief Error information when the processor is aborted.
 */
struct ErrorInformation {
  std::string message;
  Word pc;
};

/**
   \brief The state of the processor.

   In addition to registers and flags, the processor also keeps a count of the
   effective number of clock cycles that have transpired. This can be used to
   improve the realism of simulations, but this is a *logical* count only and
   does not correspond internally to any time delay.
 */
class ProcessorState : public IPrintable {
 private:
  std::vector<Word> registers_;
  std::vector<Word> specials_;
  std::bitset<3> flags_;
  std::size_t cycle_count_{0u};
  optional<ErrorInformation> error_info_{};

 public:
  /**
    When the DCPU16 is initially created, all registers are initialzed to be
    zero-valued and all flags are disabled.
   */
  explicit ProcessorState() : registers_(8u, 0u), specials_(4u, 0u) {}

  Word read(Register reg) const;  //!< Read a register.
  Word read(Special sp) const;    //!< Read a special register.
  Word read(Flag fg) const;       //!< Read a processor flag.

  void write(Register reg, Word value);  //!< Write a register.
  void write(Special sp, Word value);    //!< Write a special register.
  void write(Flag fg, bool value);       //!< Write a processor flag.

  /**
     Increase the processor clock by \p num_cyles.
   */
  inline void tickCycleCount(std::size_t num_cycles) {
    cycle_count_ += num_cycles;
  }

  /**
     Reset the processor clock to zero.
   */
  inline void clearCycleCount() { cycle_count_ = 0u; }

  /**
     Get the current value of the processor clock.
   */
  inline std::size_t cycleCount() const { return cycle_count_; }

  /**
     Set the error location and the message, for debugging.
   */
  void setError(Word pc, std::string message);

  inline optional<const ErrorInformation&> getError() const {
    if (error_info_) {
      return *error_info_;
    } else {
      return {};
    }
  }

  std::ostream& print(std::ostream& os) const override;
};

std::ostream& operator<<(std::ostream& os, const Register& reg);

std::ostream& operator<<(std::ostream& os, const Special& secial);

std::ostream& operator<<(std::ostream& os, const Flag& flag);

}  // namespace nebula

#endif  // NEBULA_PROCESSOR_STATE_HPP_
