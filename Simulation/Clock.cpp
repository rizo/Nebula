#include "Clock.hpp"

std::unique_ptr<ClockState>
Clock::run() {
    setActive();

    while ( isActive() ) {
        if ( ! _state.isOn ) {
            // Do nothing until the clock is triggered by the processor.
            _procInt->waitForTrigger();
        }

        if ( _procInt->isActive() ) {
            auto proc = _procInt->processor();
            auto a = proc->read( Register::A );
            ClockOperation op;

            switch ( a ) {
            case 0:
                handleInterrupt( ClockOperation::SetDivider, proc );
                break;
            case 1:
                handleInterrupt( ClockOperation::StoreElapsed, proc );
                break;
            }
            
            _procInt->respond();
        }

        std::this_thread::sleep_for( sim::defaults::CLOCK_BASE_PERIOD * _state.divider );
        _state.elapsed += 1;
    }

    return make_unique<ClockState>( _state );
}

void Clock::handleInterrupt( ClockOperation op, ProcessorState* proc ) {
    Word b;

    switch ( op ) {
    case ClockOperation::SetDivider:
        b = proc->read( Register::B );

        if ( b != 0 ) {
            _state.isOn = true;
            _state.divider = b;
            _state.elapsed = 0;
        } else {
            _state.isOn = false;
        }

        break;
    case ClockOperation::StoreElapsed:
        proc->write( Register::C, _state.elapsed );
        break;
    }
}
