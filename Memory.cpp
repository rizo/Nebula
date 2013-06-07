#include "Memory.hpp"

#include <algorithm>
#include <iterator>
#include <fstream>

#include <boost/spirit/include/qi.hpp>

namespace nebula {

Word Memory::read( Word offset ) {
    if ( offset >= size() ) {
        throw error::InvalidMemoryLocation {
            MemoryOperation::Read,
            offset
        };    
    }
    
    std::lock_guard<std::mutex> lock { _mutex };
    return _vec[offset];
}

void Memory::write( Word offset, Word value ) {
    if ( offset >= size() ) {
        throw error::InvalidMemoryLocation {
            MemoryOperation::Write,
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

std::shared_ptr<Memory>
Memory::fromFile( const std::string& filename, int size ) {
    namespace qi = boost::spirit::qi;

    std::ifstream file {
        filename,
        std::ios::in | std::ios::binary
    };

    if ( ! file.is_open() ) {
        throw error::MissingMemoryFile { filename };
    }

    auto begin = std::istreambuf_iterator<char> { file };
    auto end = std::istreambuf_iterator<char> {};

    std::vector<char> contents( begin, end );

    if ( contents.size() > static_cast<unsigned>( size )) {
        throw error::MemoryFileTooBig { size };
    }

    std::vector<Word> result;

    bool isSuccessful = qi::parse(
        contents.begin(),
        contents.end(),
        +qi::big_word,
        result );

    if ( ! isSuccessful ) {
        throw error::BadMemoryFile {};
    }

    auto mem = std::make_shared<Memory>( size );
    std::move( result.begin(), result.end(), mem->_vec.begin() );

    return mem;
}

}
