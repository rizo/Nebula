#include "Computer.hpp"
#include "Sdl.hpp"
#include "Simulation/Clock.hpp"
#include "Simulation/Keyboard.hpp"
#include "Simulation/Processor.hpp"
#include "Simulation/Monitor.hpp"

#include <iostream>

using namespace nebula;

int main( int, char* argv[] ) {
    google::InitGoogleLogging( argv[0] );
    sdl::initialize();

    auto memory = Memory::fromFile( argv[1], 0x10000 );
    Computer computer { memory };

    Processor proc { computer };
    Clock clock { computer };
    Monitor monitor { computer };
    Keyboard keyboard { computer };

    auto procStateF = sim::launch( proc );
    LOG( INFO ) << "Launched the processor!";

    auto clockStateF = sim::launch( clock );
    LOG( INFO ) << "Launched the clock!";

    auto monitorStateF = sim::launch( monitor );
    LOG( INFO ) << "Launched the monitor!";

    auto keyboardStateF = sim::launch( keyboard );
    LOG( INFO ) << "Launched the keyboard!";

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
