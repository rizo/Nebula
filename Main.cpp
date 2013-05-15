#include "Processor.hpp"

#include <iostream>

int main() {
    auto proc = Processor { 0x10000 };

    proc.write( 0, 0xdead );
    proc.write( 1, 0xbeef );

    address::write( proc, address::push(), 10 );
    std::cout << address::read( proc, address::peek() ) << std::endl;
    

    // std::cout << readAddress( proc, RegisterDirect { Register::A } ) << std::endl;
    // std::cout << readAddress( proc, FastDirect { 42 } ) << std::endl;
    // std::cout << readAddress( proc, Direct {} ) << std::endl;

    // writeAddress( proc, Indirect {}, 3 );
    // std::cout << proc.read( 1 ) << std::endl;
}
