#include "Processor.hpp"

#include <iostream>

int main() {
    auto proc = Processor { 0x10000 };

    proc.write( Register::A, 0xfffe );
    proc.write( Register::B, 0xfffe );
    proc.write( 0, 0x0022 );
    
    executeNext( proc );

    std::cout << proc.read( Register::B ) << std::endl;
    std::cout << proc.read( Special::Ex ) << std::endl;
    
}
