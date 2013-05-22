#include "Simulation/Processor.hpp"

#include <iostream>

int main() {

    auto memory = std::make_shared<Memory>( 0x10000 );
    auto proc = make_unique<ProcessorState>( memory );

    proc->write( Register::X, 15 );
    proc->write( Register::A, 5 );

    // Our program.
    proc->memory().write( 0, 0x0061 );
    
    sim::Processor procSim { std::move( proc ) };
    auto procFut = sim::launch( procSim );

    std::cout << "Launched the processor!" << std::endl;

    std::this_thread::sleep_for( std::chrono::seconds { 3 } );
    procSim.stop();

    procFut.get();

    std::cout << "Done!" << std::endl;
}
