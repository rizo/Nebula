#include "FloppyDrive.hpp"

#include <algorithm>

DEFINE_LOGGER( FLOPPY, "Floppy" )

namespace nebula {

Sector& FloppyDriveState::getSector( Word index ) {
    int trackIndex = index / sim::FLOPPY_SECTORS_PER_TRACK;
    int relativeSectorIndex = index % sim::FLOPPY_SECTORS_PER_TRACK;

    std::this_thread::sleep_for( sim::FLOPPY_TRACK_SEEK_DURATION * trackIndex );
    
    return disk[trackIndex][relativeSectorIndex];
}

FloppyDriveState::FloppyDriveState() {
    for ( auto& track : disk ) {
        for ( auto& sector : track ) {
            std::fill( std::begin( sector ), std::end( sector ), 0 );
        }
    }
}

FloppyDrive::FloppyDrive( Computer& computer ) :
    Simulation<FloppyDriveState> {},
    _computer( computer ),
    _procInt { computer.nextInterrupt( this ) },
    _memory { computer.memory() } {
}

void FloppyDrive::readToMemory( const Sector& sector, Word loc ) {
    for ( Word i = 0; i < sim::FLOPPY_WORDS_PER_SECTOR; ++i ) {
        _memory->write( loc + i, sector[i] );
        std::this_thread::sleep_for( sim::FLOPPY_WORD_ACCESS_DURATION );
    }
}

void FloppyDrive::writeFromMemory( Sector& sector, Word loc ) {
    for ( Word i = 0; i < sim::FLOPPY_WORDS_PER_SECTOR; ++i ) {
        sector[i] = _memory->read( loc + i );
        std::this_thread::sleep_for( sim::FLOPPY_WORD_ACCESS_DURATION );
    }
}

std::unique_ptr<FloppyDriveState> FloppyDrive::run() {
    setActive();

    LOG( FLOPPY, info ) << "Simulation is active.";

    while ( isActive() ) {
        if ( _state.stateCode == FloppyDriveStateCode::Busy ) {
            // Check if an asynchronous operation has completed.
            auto status = _resultF.wait_for( std::chrono::seconds( 0 ) );

            if ( status == std::future_status::ready ) {
                LOG( FLOPPY, info ) << "Job has completed.";
                
                _resultF.get();
                _state.stateCode = FloppyDriveStateCode::Ready;
            }
        }

        if ( _procInt->isActive() ) {
            LOG( FLOPPY, info ) << "Got interrupt.";

            auto proc = _procInt->state();
            auto a = proc->read( Register::A );

            switch ( a ) {
            case 0:
                handleInterrupt( FloppyDriveOperation::Poll, proc );
                break;
            case 2:
                handleInterrupt( FloppyDriveOperation::Read, proc );
                break;
            }

            _procInt->respond();

            LOG( FLOPPY, info ) << "Handled interrupt.";
        }

        std::this_thread::sleep_for( sim::FLOPPY_SLEEP_DURATION );
    }
    
    LOG( FLOPPY, info ) << "Shutting down.";
    return {};
}

void FloppyDrive::handleInterrupt( FloppyDriveOperation op, ProcessorState* proc ) {
    Word x = proc->read( Register::X );
    Word y = proc->read( Register::Y );

    switch ( op ) {
    case FloppyDriveOperation::Poll:
        LOG( FLOPPY, info ) << "'Poll'";

        proc->write( Register::B, static_cast<Word>( _state.stateCode ) );
        proc->write( Register::C, static_cast<Word>( _state.errorCode ) );
        
        break;
    case FloppyDriveOperation::Read:
        LOG( FLOPPY, info ) << "'Read'";

        if ( (_state.stateCode == FloppyDriveStateCode::Ready)
             || (_state.stateCode == FloppyDriveStateCode::ReadyWP) ) {
            LOG( FLOPPY, info ) << format( "Reading sector %d into memory at 0x%04x" ) % x % y;

            _state.stateCode = FloppyDriveStateCode::Busy;
            // _resultF = std::async( std::launch::async,
            //                        [&] {
            //                            readToMemory( _state.getSector( x ), y );
            //                        });
            _resultF = std::async( std::launch::async, [] { return; } );
            proc->write( Register::B, 1 );
        } else {
            proc->write( Register::B, 0xdead );
        }

        break;
    }
}

}
