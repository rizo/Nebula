#include "Computer.hpp"
#include "Simulation/Clock.hpp"
#include "Simulation/Processor.hpp"

#include <iostream>

int main( int argc, char* argv[] ) {
    google::InitGoogleLogging( argv[0] );

    auto memory = std::make_shared<Memory>( 0x10000 );
    memory->write( 0, 0x8821 );
    memory->write( 1, 0x8c01 );
    memory->write( 2, 0x7d40 );
    memory->write( 3, 0x0008 );
    memory->write( 4, 0x8640 );
    memory->write( 5, 0x8401 );
    memory->write( 6, 0x8640 );
    memory->write( 7, 0xa381 );
    memory->write( 8, 0x8862 );
    memory->write( 9, 0x8560 );

    Computer computer { memory };

    Processor proc { computer };
    Clock clock { computer };

    auto procStateF = sim::launch( proc );
    LOG( INFO ) << "Launched the processor!";

    auto clockStateF = sim::launch( clock );
    LOG( INFO ) << "Launched the clock!";


    std::this_thread::sleep_for( std::chrono::milliseconds { 10000 } );
    proc.stop();
    clock.stop();

    auto procState = procStateF.get();

    std::cout << format( "0x%04x" ) % procState->read( Register::X ) << std::endl;
    
    clockStateF.get();
}
