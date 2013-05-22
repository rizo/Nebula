#include "Processor.hpp"

#include <thread>

namespace sim {

void Processor::run() {
    while ( true ) {
        executeNext( *_proc );

        std::this_thread::sleep_for( _tickDuration * _proc->clock() );
        _proc->clearClock();
    }
}

}
