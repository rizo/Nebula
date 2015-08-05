/**
   \file decode.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Decode instructions, opcodes, and addresses.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#ifndef NEBULA_DECODE_HPP_
#define NEBULA_DECODE_HPP_

#pragma once

#include "instruction.hpp"
#include "prelude.hpp"

namespace nebula {

namespace error {

/**
   \brief The Word could not be decoded.
 */
class MalformedWord : public std::invalid_argument {
 public:
  explicit MalformedWord(Word word)
      : std::invalid_argument{
            (format("Malformed word: 0x%04x.") % word).str()} {}
};

}  // namespace error

/**
   \brief Address position.

   In the instruction

   \code{.asm}
   SET    [X], 42
   \endcode

   then AddressContext::A corresponds to \p 42 and AddressContext::B corresponds
   to \p [X].
 */
enum class AddressContext {
  A,
  B
};

/**
   \brief Try to decode an opcode.
 */
optional<OpCode> decodeOpCode(Word code);

/**
   \brief Try to decode a special opcode.
 */
optional<SpecialOpCode> decodeSpecialOpCode(Word code);

/**
   \brief Try to decode an address.
   
   If a valid address cannot not be decoded, then return \p nullptr.
 */
unique_ptr<IAddress> decodeAddress(AddressContext context, Word code);

/**
   \brief Try to decode an instruction.

   If a valid instruction cannot be decoded, then return \p nullptr.
 */
unique_ptr<IInstruction> decodeInstruction(Word word);

}  // namespace nebula

#endif  // NEBULA_DECODE_HPP_
