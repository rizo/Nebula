#include "Simulation/Processor.hpp"

#include <iostream>
#include <thread>

int main() {

    auto memory = std::make_shared<Memory>( 0x10000 );
    auto proc = make_unique<ProcessorState>( memory );

    proc->write( Register::X, 15 );
    proc->write( Register::A, 5 );

    // Our program.
    proc->memory().write( 0, 0x8461 );
    proc->memory().write( 1, 0x9862 );
    proc->memory().write( 2, 0x8b81 );
    

    sim::Processor procSim { std::move( proc ) };

    std::thread th { [&] { procSim.run(); } };

    std::cout << "Launched the processor!" << std::endl;

    th.join();
}
