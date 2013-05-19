#include "Memory.hpp"
#include "Processor.hpp"

#include <iostream>

int main() {
    auto memory = std::make_shared<Memory>( 0x10000 );
    Processor proc { memory };

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
