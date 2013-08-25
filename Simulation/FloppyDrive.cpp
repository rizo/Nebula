// Simulation/FloppyDrive.cpp
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

#include "FloppyDrive.hpp"

#include <algorithm>
#include <random>

DEFINE_LOGGER( FLOPPY, "Floppy" )

namespace nebula {

FloppyDriveState::FloppyDriveState() :
    disk( Disk( sim::FLOPPY_TRACKS_PER_DISK,
                 Track( sim::FLOPPY_SECTORS_PER_TRACK,
                        Sector( sim::FLOPPY_WORDS_PER_SECTOR, 0 ) ) ) ) {

    std::random_device rd;
    std::mt19937 gen { rd() };
    std::uniform_real_distribution<> dist( 0, 1 );
    
    for ( int i = 0; i < sim::FLOPPY_SECTORS_PER_DISK; ++i ) {
        sectorErrors[i] = dist( gen ) <= sim::FLOPPY_SECTOR_ERROR_PROBABILITY;
    }
}

FloppyDrive::FloppyDrive( Computer& computer ) :
    Simulation<FloppyDriveState> {},
    _computer( computer ),
    _procInt { computer.nextInterrupt( this ) },
    _memory { computer.memory() } {
}

Sector& FloppyDrive::getSector( Word index ) {
    int trackIndex = index / sim::FLOPPY_SECTORS_PER_TRACK;
    int relativeSectorIndex = index % sim::FLOPPY_SECTORS_PER_TRACK;

    Sector& sector = _state.disk[trackIndex][relativeSectorIndex];
    std::this_thread::sleep_for( sim::FLOPPY_TRACK_SEEK_DURATION * trackIndex );

    if ( _state.sectorErrors[index] ) {
        _state.errorCode = FloppyDriveErrorCode::BadSector;
        sendInterruptIfEnabled();

        // Zero out the sector.
        std::fill( std::begin( sector ), std::end( sector ), 0 );
    }

    return sector;
}

std::unique_ptr<FloppyDriveState> FloppyDrive::run() {
    setActive();

    LOG( FLOPPY, info ) << "Simulation is active.";

    while ( isActive() ) {
        if ( _isReading ) {
            if ( _readF.wait_for( std::chrono::seconds { 0 } ) == std::future_status::ready ) {
                LOG( FLOPPY, info ) << "Reading has completed.";
                
                _readF.get();
                _state.stateCode = FloppyDriveStateCode::Ready;
                _isReading = false;
                sendInterruptIfEnabled();
            }
        } else if ( _isWriting ) {
            if ( _writeF.wait_for( std::chrono::seconds { 0 } ) == std::future_status::ready ) {
                LOG( FLOPPY, info ) << "Writing has completed.";

                auto result = _writeF.get();
                getSector( result.first ) = result.second;
                _state.stateCode = FloppyDriveStateCode::Ready;
                _isWriting = false;
                sendInterruptIfEnabled();
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
            case 1:
                handleInterrupt( FloppyDriveOperation::EnableInterrupts, proc );
                break;
            case 2:
                handleInterrupt( FloppyDriveOperation::Read, proc );
                break;
            case 3:
                handleInterrupt( FloppyDriveOperation::Write, proc );
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

void FloppyDrive::sendInterruptIfEnabled() {
    if ( _state.interruptsEnabled ) {
        _computer.queue().push( _state.message );
    }
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
    case FloppyDriveOperation::EnableInterrupts:
        LOG( FLOPPY, info ) << "'EnableInterrupts'";

        if ( x != 0 ) {
            _state.interruptsEnabled = true;
            _state.message = x;
        } else {
            _state.interruptsEnabled = false;
        }

        break;
    case FloppyDriveOperation::Read:
        LOG( FLOPPY, info ) << "'Read'";

        sendInterruptIfEnabled();

        if ( (_state.stateCode == FloppyDriveStateCode::Ready)
             || (_state.stateCode == FloppyDriveStateCode::ReadyWP) ) {
            LOG( FLOPPY, info ) << format( "Reading sector %d into memory at 0x%04x" ) % x % y;

            _state.stateCode = FloppyDriveStateCode::Busy;

            Sector sector = getSector( x );
            auto loc = y;
            auto mem = _memory;

            _readF = std::async( std::launch::async,
                                   [mem, loc, sector] {
                                       for ( Word i = 0; i < sim::FLOPPY_WORDS_PER_SECTOR; ++i ) {
                                           mem->write( loc + i, sector[i] );
                                           std::this_thread::sleep_for( sim::FLOPPY_WORD_ACCESS_DURATION );
                                       }
                                   } );

            proc->write( Register::B, 1 );
            _isReading = true;
        } else {
            proc->write( Register::B, 0xdead );
            _state.errorCode = FloppyDriveErrorCode::Busy;
        }

        break;

    case FloppyDriveOperation::Write:
        LOG( FLOPPY, info ) << "'Write'";

        sendInterruptIfEnabled();

        if ( _state.stateCode == FloppyDriveStateCode::Ready ) {
            LOG( FLOPPY, info ) << format( "Writing sector %d from memory at 0x%04x" ) % x % y;

            _state.stateCode = FloppyDriveStateCode::Busy;

            auto loc = y;
            auto mem = _memory;

            _writeF = std::async( std::launch::async,
                                  [mem, loc, x] {
                                      Sector sector;
                                      
                                      for ( Word i = 0; i < sim::FLOPPY_WORDS_PER_SECTOR; ++i ) {
                                          sector.push_back( mem->read( loc + i ) );
                                          std::this_thread::sleep_for( sim::FLOPPY_WORD_ACCESS_DURATION );
                                      }
                                      
                                      return std::make_pair( x, sector );
                                  } );
            proc->write( Register::B, 1 );
            _isWriting = true;
        } else {
            proc->write( Register::B, 0xdead );
            _state.errorCode = FloppyDriveErrorCode::Busy;
        }

        break;
    }
}

}
