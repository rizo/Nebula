#include "Processor.hpp"

#include <thread>

namespace sim {

std::unique_ptr<ProcessorState>
Processor::run() {
    setActive();

    while ( isActive() ) {
        executeNext( *_proc );

        // Check to see if an interrupt was called to a HW device.
        auto index = _proc->takeInterruptIndex();

        if ( index ) {
            auto inter = _computer.interruptByIndex( *index );
            
            // Trigger the interrupt, and wait for the device to respond.
            inter->trigger( std::move( _proc ) );
            _proc = inter->waitForResponse();

            std::cout << "Processor got response from hardware!" << std::endl;
        }

        std::this_thread::sleep_for( _tickDuration * _proc->clock() );
        _proc->clearClock();
    }

    return std::move( _proc );
}

}
