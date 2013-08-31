// Main.cpp
//
// Copyright 2013 Jesse Haber-Kucharsky
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Computer.hpp"
#include "Sdl.hpp"
#include "Simulation/Clock.hpp"
#include "Simulation/FloppyDrive.hpp"
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
        ( "verbose,v", "Output verbose logging information to the console." )
        ( "floppy,f", "Insert a floppy disk into the drive before the simulation starts." )
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
        std::cout << "This is Nebula, the DCPU-16 emulator." << std::endl;
        std::cout << "Copyright 2013 Jesse Haber-Kucharsky" << std::endl;
        std::cout << std::endl;
        std::cout << "Usage: nebula [OPTIONS] memory-file" << std::endl;
        std::cout << std::endl;
        std::cout << visible << std::endl;
        return EXIT_SUCCESS;
    }

    logging::initialize( vm.count( "verbose" ) != 0,
                         logging::Severity::info );
    sdl::initialize();


    auto byteOrder = vm.count( "little-endian" ) ? ByteOrder::LittleEndian : ByteOrder::BigEndian;

    auto memory = Memory::fromFile( vm["memory-file"].as<std::string>(),
                                    0x10000,
                                    byteOrder );
    Computer computer { memory };

    Clock clock { computer };
    Monitor monitor { computer };
    Keyboard keyboard { computer };
    FloppyDrive floppy { computer };

    if ( vm.count( "floppy" ) ) {
        floppy.insertDisk( false );
    }

    Processor proc { computer };

    auto procStateF = sim::launch( proc );
    LOG( MAIN, info ) << "Launched the processor!";

    auto clockStateF = sim::launch( clock );
    LOG( MAIN, info ) << "Launched the clock!";

    auto monitorStateF = sim::launch( monitor );
    LOG( MAIN, info ) << "Launched the monitor!";

    auto keyboardStateF = sim::launch( keyboard );
    LOG( MAIN, info ) << "Launched the keyboard!";

    auto floppyStateF = sim::launch( floppy );
    LOG( MAIN, info ) << "Launched the floppy drive!";

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
    floppy.stop();
    clock.stop();
    monitor.stop();
    keyboard.stop();

    // It is important to get this processor state first, in case it
    // has thrown an exception. If it has, then getting the state of
    // any of the peripherals can be stalled while waiting for a
    // response from the non-running processor and the program itself
    // will be unresponsive.
    auto state = procStateF.get();
    clockStateF.get();
    monitorStateF.get();
    keyboardStateF.get();
    floppyStateF.get();

    dumpToLog( *state );
}
