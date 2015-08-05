/**
   \file interactive.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief User interaction with the emulator.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#ifndef NEBULA_INTERACTIVE_HPP_
#define NEBULA_INTERACTIVE_HPP_

#pragma once

#include "prelude.hpp"
#include "processor-state.hpp"

namespace nebula {

// Forward declaration.
class Computer;

/**
   \brief Types of interactions.
 */
namespace interactive {

/**
   \brief Respond affirmatively a fixed number of times.
 */
class CountDown {
 private:
  std::size_t count_down_;

 public:
  explicit CountDown(std::size_t times) : count_down_{times} {}

  /**
     \brief \p true until it's been called the specified number of times.
   */
  bool operator()(const Computer&) { return --count_down_ == 0u; }
};

/**
   \brief Halt execution of the DCPU16 under a condition.
 */
class ConditionalHalt {
 private:
  std::function<bool(const Computer&)> condition_;

 public:
  explicit ConditionalHalt()
      : condition_{[](const Computer&) { return false; }} {}

  explicit ConditionalHalt(std::function<bool(const Computer&)> condition)
      : condition_{condition} {}

  /**
     \p true if the condition is met.
   */
  bool doHalt(const Computer& computer) { return condition_(computer); }
};

/**
   \brief Step \p num_instructions.
 */
ConditionalHalt stepHalt(std::size_t num_instructions);

/**
   \brief Continue execution until a break-point or another interruption.
 */
ConditionalHalt continueHalt();

/**
   \brief A break in execution of the DCPU16, at a fixed program location.

   A break-point occurs at a certain address in the program that the DCPU16 is
   exeucting. If execution reaches that point, then it is halted.

   Break-points each have an index, which should be unique but the onus for
   maintaining this uniqueness is on the caller.
 */
class BreakPoint {
 private:
  Word offset_;
  std::size_t index_;

 public:
  explicit BreakPoint(Word offset, std::size_t index)
      : offset_{offset}, index_{index} {}

  /**
     \brief \p true if the program has reached this break-point.
   */
  bool matches(const Computer& computer) const;

  inline Word offset() const { return offset_; }

  inline std::size_t index() const { return index_; }
};

struct ShowBreak {};

struct ShowState {};

struct ShowSource {
  std::size_t num_instructions;
};

struct Quit {};

/**
   \brief A command that can be entered by the user.
 */
using Command = boost::variant<BreakPoint,
                               ConditionalHalt,
                               ShowState,
                               ShowSource,
                               ShowBreak,
                               Quit>;

/**
   \brief Read a command interactively from the user on stdin.
 */
Command waitForCommand(const ProcessorState& processor_state);

}  // namespace interactive

}  // namespace nebula

#endif  // NEBULA_INTERACTIVE_HPP_
