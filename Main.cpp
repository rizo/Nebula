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

    auto decoded  = decode<Instruction>( 0x4fe1 );
    if ( ! decoded ) {
        std::cout << "Failed to decode!" << std::endl;
    } else {
        instruction::Binary binIns = boost::get<instruction::Binary>( *decoded );

        std::cout << "Decode successful!" << std::endl;
    }
}
