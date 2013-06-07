#pragma once

#include "Fundamental.hpp"

#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>

namespace nebula {

enum class MemoryOperation {
    Read, Write
};

namespace error {

class InvalidMemoryLocation : public std::out_of_range {
    MemoryOperation _operation;
    Word _location;
public:
    explicit InvalidMemoryLocation( MemoryOperation operation, Word location ) :
        std::out_of_range {
            (format( "Cannot %s at memory location 0x%04x" )
                 % (operation == MemoryOperation::Read ? "read" : "write")
                 % location ).str()
        },
        _operation { operation },
        _location { location } {
    }

    inline MemoryOperation operation() const noexcept { return _operation; }
    inline Word location() const noexcept{ return _location; }
};

class BadMemoryFile : public std::out_of_range {
public:
    explicit BadMemoryFile() :
        std::out_of_range { "Invalid format for memory file" } {}
};

class MissingMemoryFile : public std::runtime_error {
    std::string _path;
public:
    explicit MissingMemoryFile( const std::string& path ) :
        std::runtime_error {
            (format( "Unable to open file '%s'" ) % path).str()
        },
        _path { path } {}

    const std::string& path() const noexcept { return _path; }
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

    int memorySize() const noexcept { return _memorySize; }
};

}

class Memory final {
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

}
