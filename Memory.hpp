#pragma once

#include "Fundamental.hpp"

#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>

#include <boost/format.hpp>

namespace memory {

enum class Operation {
    Read, Write
};

}

namespace error {

using boost::format;

class InvalidMemoryLocation : public std::out_of_range {
    memory::Operation _operation;
    Word _location;
public:
    explicit InvalidMemoryLocation( memory::Operation operation, Word location ) :
        std::out_of_range {
            (format( "Cannot %s at memory location 0x%04x" )
                 % (operation == memory::Operation::Read ? "read" : "write")
                 % location ).str()
        },
        _operation { operation },
        _location { location } {
    }

    inline memory::Operation operation() const { return _operation; }
    inline Word location() const { return _location; }
};

class BadMemoryFile : public std::out_of_range {
public:
    explicit BadMemoryFile() :
        std::out_of_range { "Invalid format for memory file" } {}
};

class MemoryFileTooBig : public std::out_of_range {
    int _memorySize;
public:
    explicit MemoryFileTooBig( int memorySize ) :
        std::out_of_range {
            (format( "Memory file is too big for memory (%d words)" )
                 % memorySize).str()

        },
        _memorySize { memorySize } {}

    int memorySize() const { return _memorySize; }
};

}

class Memory {
    std::vector<Word> _vec;
    std::mutex _mutex {};
public:
    static std::shared_ptr<Memory> fromFile( const std::string& filename, int size );

    explicit Memory( int size ) :
        _vec { std::vector<Word>( size ) } {}

    Word read( Word offset );
    void write( Word offset, Word value );

    int size();
};
