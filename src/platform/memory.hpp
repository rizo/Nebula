/**
   \file memory.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Volatile memory.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#ifndef NEBULA_MEMORY_HPP_
#define NEBULA_MEMORY_HPP_

#pragma once

#include "binary-units.hpp"
#include "prelude.hpp"

#include <cerrno>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <vector>

DEFINE_LOGGER(MEMORY)

namespace nebula {

/**
   \brief Nebula exceptions.
 */
namespace error {

class InvalidMemoryOffset : public std::out_of_range {
 public:
  explicit InvalidMemoryOffset(Word offset, const InformationQuantity& capacity)
      : std::out_of_range{
            (format("Attempted to access 0x%04x, but capacity is %ul.") %
             offset % capacity.value()).str()} {}
};

class BadMemoryFile : public std::invalid_argument {
 public:
  explicit BadMemoryFile(const std::string& file_name)
      : std::invalid_argument{
            (format("\"%s\" is not a valid memory file.") % file_name).str()} {}
};

class MemoryFileTooBig : public std::out_of_range {
 public:
  explicit MemoryFileTooBig(const InformationQuantity& capacity)
      : std::out_of_range{
            (format("Memory file is too big for memory with capacity {} .") %
             capacity.value()).str()} {}
};

class MemoryDump : public std::runtime_error {
 public:
  explicit MemoryDump(const fs::path& file)
      : std::runtime_error{(format("Failed to dump memory to \"%s\": %s.") %
                            file.string() % std::strerror(errno)).str()} {}
};

}  // namespace error

enum class ByteOrder {
  BigEndian,
  LittleEndian
};

/**
   \brief Simulated memory.

   Nebula's memory is arranged as a flat, word-addressable block.

   The consistency model is simple (read: stupid). Memory can be accessed and
   written to from multiple sources, but internally all requests are
   serialized. In practise, this is okay since memory is only written to from
   hardware devices when triggered to do so by a interrupt, and otherwise the
   DCPU16 computer is a uniprocessor system.
 */
class Memory {
 private:
  std::vector<Word> cells_;
  std::mutex mutex_;

 public:
  /**
     \brief Create a new empty memory block with the given capacity.
   */
  explicit Memory(const InformationQuantity& capacity);

  inline InformationQuantity capacity() const {
    return cells_.size() * units::binary::Word;
  }

  /**
     \brief Read a word of memory.

     \throws error::InvalidMemoryOffset
   */
  Word read(Word offset);

  /**
     \brief Write a word of memory.

     \throws error::InvalidMemoryOffset
   */
  void write(Word offset, Word value);

  /**
     \brief Populate the memory block with the contents of a file on disk.

     \throw error::BadMemoryFile If the file on disk cannot be read.
     \throw error::MemoryFileTooBig The size of the memory file on disk must be
       less than or equal to the capacity of the memory block when it was
       created.
   */
  void fillFromFile(const fs::path& file, ByteOrder byte_order);

  /**
     \brief Write a file on disk with the contents of the memory block.

     \throw error::MemoryDump If the file on disk cannot be opened.
   */
  void writeToFile(const fs::path& file, ByteOrder byte_order);
};

}  // namespace nebula

#endif  // NEBULA_MEMORY_HPP_
