#include "Processor.hpp"

#include <iostream>

int main() {
    auto proc = Processor { 0x10000 };

    proc.write( 0, 15 );
    proc.write( Register::A, 20 );

    Instruction ins {
        instruction::Binary {
            Opcode::Sub,
            address::registerDirect( Register::A ),
            address::direct()
        }
    };

    execute( proc, ins );

    std::cout << proc.read( Register::A ) << std::endl;
}
