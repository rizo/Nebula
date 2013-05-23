#include "Computer.hpp"
#include "Simulation/Clock.hpp"
#include "Simulation/Processor.hpp"

#include <iostream>

int main() {
    auto memory = std::make_shared<Memory>( 0x10000 );
    memory->write( 0, 0x8821 );
    memory->write( 1, 0x8640 );

    memory->write( 2, 0x0002 );
    memory->write( 3, 0x0002 );
    memory->write( 4, 0x0002 );

    memory->write( 5, 0x8801 );
    memory->write( 6, 0x8640 );
    memory->write( 7, 0xa381 );

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

    std::cout << procState.get()->read( Register::C ) << std::endl;
}
