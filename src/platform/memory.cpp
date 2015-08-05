/**
   \file memory.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "memory.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <numeric>

namespace nebula {

static void checkOffset(const Memory& memory, Word offset) {
  const auto capacity = memory.capacity();

  if ((offset * units::binary::Word) >= capacity) {
    throw error::InvalidMemoryOffset{offset, capacity};
  }
}

Memory::Memory(const InformationQuantity& capacity)
    : cells_(capacity.value(), 0u), mutex_{} {

  LOG(MEMORY, info) << format("Initialized with capacity %s words.") %
                           capacity.value();
}

Word Memory::read(Word offset) {
  std::lock_guard<std::mutex> lock{mutex_};

  checkOffset(*this, offset);
  return cells_[offset];
}

void Memory::write(Word offset, Word value) {
  std::lock_guard<std::mutex> lock{mutex_};

  checkOffset(*this, offset);
  cells_[offset] = value;
}

template <typename InputIter, typename OutputIter>
static bool convertBytesToWords(InputIter begin,
                                InputIter end,
                                ByteOrder byte_order,
                                OutputIter outputIter) {
  std::vector<std::pair<std::uint8_t, std::uint8_t>> pairs;
  InputIter iter = begin;
  while (iter != end) {
    if ((iter + 1u) == end) {
      return false;
    }

    pairs.emplace_back(std::make_pair(*iter, *(iter + 1)));
    iter += 2u;
  }

  std::transform(
      std::begin(pairs),
      std::end(pairs),
      outputIter,
      [&byte_order](const std::pair<std::uint8_t, std::uint8_t>& pair) {
        switch (byte_order) {
          case ByteOrder::LittleEndian:
            return (pair.second << 8u) | pair.first;

          case ByteOrder::BigEndian:
            return (pair.first << 8u) | pair.second;
        }

        // GCC is stupid... This is never reached.
        return 0xdead;
      });

  return true;
}

void Memory::fillFromFile(const fs::path& file, ByteOrder byte_order) {
  std::ifstream stream{file.string(), std::ios::in | std::ios::binary};

  if (!stream.is_open()) {
    throw error::BadMemoryFile{file.string()};
  }

  auto stream_begin = std::istreambuf_iterator<char>{stream};
  auto stream_end = std::istreambuf_iterator<char>{};

  std::vector<char> file_contents{stream_begin, stream_end};
  std::vector<Word> memory_cells;

  if (!convertBytesToWords(std::begin(file_contents),
                           std::end(file_contents),
                           byte_order,
                           std::back_inserter(memory_cells))) {
    throw error::BadMemoryFile{file.string()};
  }

  auto file_size = memory_cells.size() * units::binary::Word;

  if (file_size > capacity()) {
    throw error::MemoryFileTooBig{capacity()};
  }

  LOG(MEMORY, info) << format("Read %s words from \"%s\".") %
                           file_size.value() % file.string();

  // Move the new memory cells into the memory object itself.
  std::move(memory_cells.begin(), memory_cells.end(), cells_.begin());
}

void Memory::writeToFile(const fs::path& file, ByteOrder byte_order) {
  LOG(MEMORY, info) << format("Dumping memory to \"%s\".") % file.string();

  std::ofstream stream{file.string(), std::ios::out | std::ios::binary};
  if (!stream.is_open()) {
    throw error::MemoryDump{file};
  }

  for (const Word& w : cells_) {
    std::uint8_t first_byte;
    std::uint8_t second_byte;

    std::uint8_t most_significant_byte = (w & 0xff00) >> 8u;
    std::uint8_t least_significant_byte = w & 0x00ff;

    switch (byte_order) {
      case ByteOrder::LittleEndian:
        first_byte = least_significant_byte;
        second_byte = most_significant_byte;
        break;

      case ByteOrder::BigEndian:
        first_byte = most_significant_byte;
        second_byte = least_significant_byte;
        break;
    }

    stream << first_byte << second_byte;
  }
}

}  // namespace nebula
