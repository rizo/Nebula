#include "Computer.hpp"
#include "Simulation/Clock.hpp"
#include "Simulation/Processor.hpp"

#include <iostream>

int main() {
    auto memory = std::make_shared<Memory>( 0x10000 );
    memory->write( 0, 0xac61 );
    memory->write( 1, 0x0e40 );

    Computer computer { memory };

    sim::Processor procSim { computer };
    sim::Clock clockSim { computer };

    auto procFut = sim::launch( procSim );
    std::cout << "Launched the processor!" << std::endl;

    auto clockFut = sim::launch( clockSim );
    std::cout << "Launched the clock!" << std::endl;

    std::this_thread::sleep_for( std::chrono::milliseconds { 250 } );
    procSim.stop();
    clockSim.stop();

    procFut.get();
    clockFut.get();

    std::cout << "Done!" << std::endl;
}
