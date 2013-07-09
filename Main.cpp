#include "Computer.hpp"
#include "Sdl.hpp"
#include "Simulation/Clock.hpp"
#include "Simulation/Keyboard.hpp"
#include "Simulation/Processor.hpp"
#include "Simulation/Monitor.hpp"

#include <iostream>

using namespace nebula;

DEFINE_LOGGER( MAIN, "Main" )

int main( int, char* argv[] ) {
    logging::initialize( true, logging::Severity::info );
    sdl::initialize();

    auto memory = Memory::fromFile( argv[1], 0x10000, ByteOrder::BigEndian );
    Computer computer { memory };

    Processor proc { computer };
    Clock clock { computer };
    Monitor monitor { computer };
    Keyboard keyboard { computer };

    auto procStateF = sim::launch( proc );
    LOG( MAIN, info ) << "Launched the processor!";

    auto clockStateF = sim::launch( clock );
    LOG( MAIN, info ) << "Launched the clock!";

    auto monitorStateF = sim::launch( monitor );
    LOG( MAIN, info ) << "Launched the monitor!";

    auto keyboardStateF = sim::launch( keyboard );
    LOG( MAIN, info ) << "Launched the keyboard!";

    SDL_Event event;
    while ( true ) {
        if ( SDL_PollEvent( &event ) ) {
            if ( event.type == SDL_QUIT ) {
                break;
            } else if ( event.type == SDL_KEYDOWN ) {
                keyboard.state().setKey( &event.key.keysym );
            }
        }

        if ( sim::isReady( procStateF ) ) {
            break;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds { 10 } );
    }

    proc.stop();
    clock.stop();
    monitor.stop();
    keyboard.stop();

    clockStateF.get();
    monitorStateF.get();
    keyboardStateF.get();

    auto state = procStateF.get();
    dumpToLog( *state );
}
