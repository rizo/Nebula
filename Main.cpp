#include "Memory.hpp"
#include "Processor.hpp"

#include <iostream>

int main() {
    auto memory = std::make_shared<Memory>( 0x10000 );
    Processor proc { memory };

    proc.write( Register::X, 15 );
    proc.write( Register::A, 5 );

    proc.memory().write( 0, 0x0062 );
    executeNext( proc );

    std::cout << proc.read( Register::X ) << std::endl;
}
