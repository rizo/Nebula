#include "Computer.hpp"
#include "Simulation/Clock.hpp"
#include "Simulation/Processor.hpp"

#include <iostream>

int main( int argc, char* argv[] ) {
    google::InitGoogleLogging( argv[0] );

    auto memory = std::make_shared<Memory>( 0x10000 );
    memory->write( 0, 0x8620 );
    memory->write( 1, 0x8b81 );

    Computer computer { memory };

    Processor proc { computer };
    Clock clock { computer };

    auto procStateF = sim::launch( proc );
    LOG( INFO ) << "Launched the processor!";

    auto clockStateF = sim::launch( clock );
    LOG( INFO ) << "Launched the clock!";

    std::this_thread::sleep_for( std::chrono::milliseconds { 500 } );
    proc.stop();
    clock.stop();

    auto procState = procStateF.get();

    std::cout << format( "0x%04x" ) % procState->read( Register::A ) << std::endl;
    std::cout << format( "0x%04x" ) % procState->read( Register::B ) << std::endl;

    std::cout << std::endl;

    std::cout << format( "0x%04x" ) % procState->read( Register::X ) << std::endl;
    std::cout << format( "0x%04x" ) % procState->read( Register::Y ) << std::endl;

    
    std::cout << std::endl;

    std::cout << format( "0x%04x" ) % procState->read( Register::C ) << std::endl;
    
    clockStateF.get();
}
