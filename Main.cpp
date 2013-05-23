#include "Computer.hpp"
#include "Simulation/Clock.hpp"
#include "Simulation/Processor.hpp"

#include <iostream>

int main() {
    auto memory = std::make_shared<Memory>( 0x10000 );
    memory->write( 0, 0x8461 );
    memory->write( 1, 0x0e40 );

    Computer computer { memory };


    Processor proc { computer };
    Clock clock { computer };

    auto procState = sim::launch( proc );
    std::cout << "Launched the processor!" << std::endl;

    auto clockState = sim::launch( clock );
    std::cout << "Launched the clock!" << std::endl;

    std::this_thread::sleep_for( std::chrono::milliseconds { 250 } );
    proc.stop();
    clock.stop();

    procState.get();
    clockState.get();

    std::cout << "Done!" << std::endl;
}
