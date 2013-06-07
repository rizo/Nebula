#include "Processor.hpp"

#include <thread>

namespace nebula {

std::unique_ptr<ProcessorState>
Processor::run() {
    setActive();

    LOG( PROC, info ) << "Simulation is active.";

    while ( isActive() ) {
        auto now = std::chrono::system_clock::now();

        _proc->executeNext();

        if ( auto ins = dynamic_cast<const instruction::Unary*>( _proc->lastInstruction() ) ) {
            executeSpecial( ins );
        }

        std::this_thread::sleep_until( now + (_tickDuration * _proc->clock()) );
        _proc->clearClock();

        if ( _computer.queue().hasInterrupt() &&
             _computer.ia() != 0 &&
             ! _computer.onlyQueuing() ) {
            handleInterrupt();
        }
    }

    LOG( PROC, info ) << "Shutting down.";
    return std::move( _proc );
}

void Processor::handleInterrupt() {
    auto msg = _computer.queue().pop();
    auto push = mode::Push {};

    LOG( PROC, info ) << format( "Handling HW interrupt of 0x%04x." ) % msg;
    
    _computer.setOnlyQueuing( true );
    push.store( *_proc, _proc->read( Special::Pc ) );
    push.store( *_proc, _proc->read( Register::A ) );
    _proc->write( Special::Pc, _computer.ia() );
    _proc->write( Register::A, msg );
}

void Processor::executeSpecial( const instruction::Unary* ins ) {
    auto load = [this, ins] { return ins->address->load( *_proc ); };
    auto store = [this, ins] ( Word value ) { ins->address->store( *_proc, value ); };

    if ( ins->opcode == SpecialOpcode::Hwi ) {
        Word index = load();
        auto inter = _computer.interruptByIndex( index );

        // Trigger the interrupt, and wait for the device to respond.
        inter->trigger( std::move( _proc ) );
        _proc = inter->waitForResponse();
    } else if ( ins->opcode == SpecialOpcode::Hwn ) {
        store( _computer.numDevices() );
        _proc->tickClock( 2 );
    } else if ( ins->opcode == SpecialOpcode::Hwq ) {
        Word index = load();
        auto info = _computer.infoByIndex( index );

        _proc->write( Register::A, info.id.value & 0xffff );
        _proc->write( Register::B, (info.id.value & 0xffff0000) >> 16 );

        _proc->write( Register::X, info.manufacturer.value & 0xffff );
        _proc->write( Register::Y, (info.manufacturer.value & 0xffff0000) >> 16 );

        _proc->write( Register::C, info.version.value );
    } else if ( ins->opcode == SpecialOpcode::Iag ) {
        store( _computer.ia() );
    } else if ( ins->opcode == SpecialOpcode::Ias ) {
        auto value = load();
        _computer.setIa( value );

        if ( value == 0 ) {
            LOG( PROC, warning ) << "Ignoring incoming HW interrupts.";
            _computer.queue().setEnabled( false );
        } else {
            LOG( PROC, info ) << "Enabling incoming HW interrupts.";
            _computer.queue().setEnabled( true );
        }
    } else if ( ins->opcode == SpecialOpcode::Rfi ) {
        auto pop = mode::Pop {};
        _proc->write( Register::A, pop.load( *_proc ) );
        _proc->write( Special::Pc, pop.load( *_proc ) );
        _computer.setOnlyQueuing( false );
    } else if ( ins->opcode == SpecialOpcode::Iaq ) {
        _computer.setOnlyQueuing( load() != 0 );
    }
}

}
