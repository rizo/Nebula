#include "Memory.hpp"

Word Memory::read( Word offset ) {
    if ( offset >= size() ) {
        throw error::InvalidMemoryLocation {
            memory::Operation::Read,
            offset
        };    
    }
    
    std::lock_guard<std::mutex> lock { _mutex };
    return _vec[offset];
}

void Memory::write( Word offset, Word value ) {
    if ( offset >= size() ) {
        throw error::InvalidMemoryLocation {
            memory::Operation::Write,
            offset
        };
    }

    std::lock_guard<std::mutex> lock { _mutex };
    _vec[offset] = value;
}

int Memory::size() {
    std::lock_guard<std::mutex> lock { _mutex };
    return _vec.size();
}
