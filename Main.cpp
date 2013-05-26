#include "Computer.hpp"
#include "Simulation/Clock.hpp"
#include "Simulation/Processor.hpp"

#include <iostream>

int main( int argc, char* argv[] ) {
    google::InitGoogleLogging( argv[0] );

    auto memory = std::make_shared<Memory>( 0x10000 );
    memory->write( 0, 0x8821 );
    memory->write( 1, 0x0e00 );
    memory->write( 2, 0x8640 );
    memory->write( 3, 0x9381 );

    Computer computer { memory };

    Processor proc { computer };
    Clock clock { computer };

    auto procState = sim::launch( proc );
    LOG( INFO ) << "Launched the processor!";

    auto clockState = sim::launch( clock );
    LOG( INFO ) << "Launched the clock!";

    std::this_thread::sleep_for( std::chrono::milliseconds { 500 } );
    proc.stop();
    clock.stop();

    std::cout << procState.get()->read( Register::X ) << std::endl;
    clockState.get();
}
