#include "Computer.hpp"
#include "Simulation/Clock.hpp"
#include "Simulation/Processor.hpp"

#include <iostream>

int main( int argc, char* argv[] ) {
    google::InitGoogleLogging( argv[0] );

    auto memory = Memory::fromFile( "/home/jesse/foo.bin", 0x10000 );
    Computer computer { memory };

    Processor proc { computer };
    Clock clock { computer };

    auto procStateF = sim::launch( proc );
    LOG( INFO ) << "Launched the processor!";

    auto clockStateF = sim::launch( clock );
    LOG( INFO ) << "Launched the clock!";

    std::this_thread::sleep_for( std::chrono::milliseconds { 5000 } );
    proc.stop();
    clock.stop();

    auto procState = procStateF.get();

    std::cout << format( "0x%04x" ) % procState->read( Register::X ) << std::endl;
    
    clockStateF.get();
}
