#include "Clock.hpp"

std::unique_ptr<ClockState>
Clock::run() {
    setActive();

    LOG( INFO ) << "Clock simulation is active.";

    while ( isActive() ) {
        if ( ! _state.isOn ) {
            // Do nothing until the clock is triggered by the processor.
            LOG( INFO ) << "Clock is off. waiting for interrupt.";

            _procInt->waitForTrigger();
        }

        if ( _procInt->isActive() ) {
            LOG( INFO ) << "Clock got interrupt.";

            auto proc = _procInt->state();
            auto a = proc->read( Register::A );

            switch ( a ) {
            case 0:
                handleInterrupt( ClockOperation::SetDivider, proc );
                break;
            case 1:
                handleInterrupt( ClockOperation::StoreElapsed, proc );
                break;
            case 2:
                handleInterrupt( ClockOperation::EnableInterrupts, proc );
                break;
            }
            
            _procInt->respond();

            LOG( INFO ) << "Clock handled interrupt.";
        }

        std::this_thread::sleep_for( sim::CLOCK_BASE_PERIOD * _state.divider );
        _state.elapsed += 1;

        if ( _state.interruptsEnabled ) {
            _computer.queue().push( _state.message );
        }
    }

    LOG( INFO ) << "Clock simulation shutting down.";
    return make_unique<ClockState>( _state );
}

void Clock::handleInterrupt( ClockOperation op, ProcessorState* proc ) {
    Word b;

    switch ( op ) {
    case ClockOperation::SetDivider:
        LOG( INFO ) << "Clock executing 'SetDivider'";

        b = proc->read( Register::B );

        if ( b != 0 ) {
            LOG( INFO ) << "Turning on clock with divider " << b << ".";

            _state.isOn = true;
            _state.divider = b;
            _state.elapsed = 0;
        } else {
            LOG( WARNING ) << "Turning clock off";
            _state.isOn = false;
        }

        break;
    case ClockOperation::StoreElapsed:
        LOG( INFO ) << "Clock executing 'StoreElapsed'";

        proc->write( Register::C, _state.elapsed );
        break;
    case ClockOperation::EnableInterrupts:
        LOG( INFO ) << "Clock executing 'EnableInterrupts'";

        b = proc->read( Register::B );

        if ( b != 0 ) {
            _state.interruptsEnabled = true;
            _state.message = b;
        } else {
            LOG( WARNING ) << "Turning interrupts off.";
            _state.interruptsEnabled = false;
        }
    }
}
