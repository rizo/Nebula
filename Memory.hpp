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

}

class Memory {
    std::vector<Word> _vec;
    std::mutex _mutex {};
public:
    explicit Memory( int size ) :
        _vec { std::vector<Word>( size ) } {}

    Word read( Word offset );
    void write( Word offset, Word value );

    int size();
};
