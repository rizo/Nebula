/**
   \file interactive.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "interactive.hpp"

#include "computer.hpp"

#include <boost/tokenizer.hpp>

#include <deque>
#include <stack>

namespace nebula {

namespace interactive {

ConditionalHalt stepHalt(std::size_t num_instructions) {
  return ConditionalHalt{CountDown{num_instructions}};
}

static bool alwaysFalse(const Computer&) { return false; }

ConditionalHalt continueHalt() {
  return ConditionalHalt{&alwaysFalse};
}

bool BreakPoint::matches(const Computer& computer) const {
  return computer.state().read(Special::Pc) == offset_;
}

using TokenStream = std::stack<std::string>;

static TokenStream tokenize(const std::string& line) {
  boost::char_separator<char> separator{" "};
  boost::tokenizer<boost::char_separator<char>> tokens{line, separator};

  std::deque<std::string> deque;
  for (const auto& token : tokens) {
    deque.emplace_front(token);
  }

  return TokenStream{deque};
}

namespace parse {

static bool word(TokenStream& stream, const std::string& word) {
  if (stream.empty()) {
    return false;
  }

  if (stream.top() == word) {
    stream.pop();
    return true;
  }

  return false;
}

static optional<nebula::Word> integer(TokenStream& stream) {
  if (stream.empty()) {
    return {};
  }

  unsigned long result;

  try {
    auto top = stream.top();
    if ((top.size() >= 2) && (top.substr(0, 2) == "0x")) {
      result = std::stoul(top, nullptr, 16);
    } else {
      result = std::stoul(top, nullptr);
    }

    stream.pop();
    return result;
  }
  catch (std::invalid_argument&) {
    return {};
  }
}

static optional<ShowBreak> showBreakCmd(TokenStream& stream) {
  if (word(stream, "break")) {
    return ShowBreak{};
  } else {
    return {};
  }
}

static optional<ShowState> showStateCmd(TokenStream& stream) {
  if (word(stream, "state")) {
    return ShowState{};
  } else {
    return {};
  }
}

static optional<ShowSource> showSourceCmd(TokenStream& stream) {
  if (!word(stream, "source")) {
    return {};
  }

  if (stream.empty()) {
    return ShowSource{5u};
  } else {
    auto num_instructions = integer(stream);
    if (num_instructions) {
      return ShowSource{*num_instructions};
    } else {
      std::cout << "Expected an integer number of instructions to show."
                << std::endl;
      return {};
    }
  }
}

static optional<Command> showCmd(TokenStream& stream) {
  std::string literal{"show"};
  if (!word(stream, literal)) {
    return {};
  }

  optional<Command> result;

  result = showBreakCmd(stream);
  if (result) return result;

  result = showStateCmd(stream);
  if (result) return result;

  result = showSourceCmd(stream);
  if (result) return result;

  stream.emplace(literal);
  std::cout << "Expected \"show break\"\n"
            << "      or \"show state\"\n"
            << "      or \"show source [num-instructions]\"\n";
  return {};
}

static optional<ConditionalHalt> stepCmd(TokenStream& stream) {
  if (!word(stream, "step")) {
    return {};
  }

  if (stream.empty()) {
    return stepHalt(1u);
  } else {
    auto num_instructions = integer(stream);
    if (num_instructions) {
      return stepHalt(*num_instructions);
    } else {
      std::cout << "Expected an integer number of instructions to step through."
                << std::endl;
      return {};
    }
  }
}

static optional<Quit> quitCmd(TokenStream& stream) {
  if (word(stream, "quit")) {
    return Quit{};
  } else {
    return {};
  }
}

static optional<ConditionalHalt> continueCmd(TokenStream& stream) {
  if (word(stream, "continue")) {
    return continueHalt();
  } else {
    return {};
  }
}

static ::nebula::Word gBreakPointIndex = 0u;

static optional<BreakPoint> breakCmd(TokenStream& stream) {
  if (!word(stream, "break")) {
    return {};
  }

  auto offset = integer(stream);
  if (offset) {
    return BreakPoint{*offset, ++gBreakPointIndex};
  }

  std::cout << "Expected an offset into memory." << std::endl;
  return {};
}

static void showHelp() {
  std::cout << "Valid commands are:\n"
            << "  show\n"
            << "    source [num-instructions]\n"
            << "    break\n"
            << "    state\n"
            << "  break <offset>\n"
            << "  step [num-instructions]\n"
            << "  continue\n"
            << "  quit" << std::endl;
}

static optional<Command> cmd(TokenStream& stream) {
  optional<Command> result;

  if (stream.empty()) {
    showHelp();
    goto bad_input;
  }

  result = continueCmd(stream);
  if (result) goto good_input;

  result = stepCmd(stream);
  if (result) goto good_input;

  result = showCmd(stream);
  if (result) goto good_input;

  result = quitCmd(stream);
  if (result) goto good_input;

  result = breakCmd(stream);
  if (result) goto good_input;

  std::cout << "Invalid command. Enter an empty command for help." << std::endl;
  goto bad_input;

good_input:
  if (!stream.empty()) {
    std::cout << "Unexpected input after command." << std::endl;
    return {};
  }

  return result;

bad_input:
  return {};
}

}  // namespace parse

Command waitForCommand(const ProcessorState& processor_state) {
  std::string line;
  optional<Command> command;

  while (true) {
    std::cout << format("[0x%04x]>>> ") % processor_state.read(Special::Pc);
    std::getline(std::cin, line);

    auto stream = tokenize(line);
    command = parse::cmd(stream);
    if (command) {
      break;
    }
  }

  return *command;
}

}  // namespace interactive

}  // namespace nebula
