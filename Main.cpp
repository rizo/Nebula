#include "Processor.hpp"

#include <iostream>

int main() {
    auto proc = Processor { 0x10000 };

    proc.write( Register::A, 20 );

    proc.write( 0, 0x7c02 );
    proc.write( 1, 500 );

    executeNext( proc );

    std::cout << proc.read( Register::A ) << std::endl;
}
