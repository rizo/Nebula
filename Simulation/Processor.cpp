#include "Processor.hpp"

#include <thread>

namespace sim {

std::unique_ptr<ProcessorState>
Processor::run() {
    setActive();

    while ( isActive() ) {
        executeNext( *_proc );

        std::this_thread::sleep_for( _tickDuration * _proc->clock() );
        _proc->clearClock();
    }

    return std::move( _proc );
}

}
