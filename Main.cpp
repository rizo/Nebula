#include "Computer.hpp"
#include "Simulation/Clock.hpp"
#include "Simulation/Processor.hpp"

#include <iostream>

int main() {
    auto memory = std::make_shared<Memory>( 0x10000 );
    memory->write( 0, 0x0e00 );
    memory->write( 1, 0x8b81 );

    Computer computer { memory };

    Processor proc { computer };
    Clock clock { computer };

    auto procState = sim::launch( proc );
    std::cout << "Launched the processor!" << std::endl;

    auto clockState = sim::launch( clock );
    std::cout << "Launched the clock!" << std::endl;

    std::this_thread::sleep_for( std::chrono::milliseconds { 500 } );
    proc.stop();
    clock.stop();

    std::cout << procState.get()->read( Register::X ) << std::endl;
    clockState.get();
}
