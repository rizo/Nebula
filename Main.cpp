#include "Computer.hpp"
#include "Sdl.hpp"
#include "Simulation/Clock.hpp"
#include "Simulation/Keyboard.hpp"
#include "Simulation/Processor.hpp"
#include "Simulation/Monitor.hpp"

#include <cstddef>
#include <iostream>

#include <boost/program_options.hpp>

using namespace nebula;

DEFINE_LOGGER( MAIN, "Main" )

int main( int argc, char* argv[] ) {
    namespace po = boost::program_options;

    po::options_description visible;
    visible.add_options()
        ( "help,h", "Produce this message." )
        ( "little-endian,e", "Assume little endian memory encoding." )
        ;

    po::options_description hidden;
    hidden.add_options()
        ( "memory-file", po::value<std::string>(), "The memory file initially loaded by the DCPU-16." )
        ;

    po::options_description desc;
    desc.add( visible ).add( hidden );

    po::positional_options_description pos;
    pos.add( "memory-file", 1 );

    po::variables_map vm {};

    try {
        po::store( po::command_line_parser( argc, argv ).
                   options( desc ).positional( pos ).run(),
                   vm );
        po::notify( vm );
    } catch ( po::error& err ) {
        std::cerr << "nebula: " << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    if ( vm.count( "help" ) || ! vm.count( "memory-file" ) ) {
        std::cout << "nebula [options] memory-file" << std::endl;
        std::cout << std::endl;
        std::cout << visible << std::endl;
        return EXIT_SUCCESS;
    }

    logging::initialize( true, logging::Severity::info );
    sdl::initialize();


    auto byteOrder = vm.count( "little-endian" ) ? ByteOrder::LittleEndian : ByteOrder::BigEndian;

    auto memory = Memory::fromFile( vm["memory-file"].as<std::string>(),
                                    0x10000,
                                    byteOrder );
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
