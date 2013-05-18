#include "Processor.hpp"

#include <iostream>

int main() {
    auto proc = Processor { 0x10000 };

    proc.write( Register::X, 15 );
    proc.write( Register::A, 5 );

    auto ins = instruction::Binary {
        Opcode::Sub,
        std::make_shared<mode::RegisterDirect>( Register::X ),
        std::make_shared<mode::RegisterDirect>( Register::A )
    };

    ins.execute( proc );

    std::cout << proc.read( Register::X ) << std::endl;

}
