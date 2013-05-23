#include "Computer.hpp"

void ProcessorInterrupt::trigger( std::unique_ptr<ProcessorState>&& proc ) {
    _isActive.store( true );
    _proc = std::move( proc );
    _condition.notify_one();
}

std::unique_ptr<ProcessorState>
ProcessorInterrupt::waitForResponse() {
    std::unique_lock<std::mutex> lock { _mutex };
    _condition.wait( lock, [&] {
            return _proc && ! isActive();
        });

    return std::move( _proc );
}

void ProcessorInterrupt::respond() {
    _isActive.store( false );
    _condition.notify_one();
}

void ProcessorInterrupt::waitForTrigger() {
    std::unique_lock<std::mutex> lock { _mutex };
    _condition.wait( lock, [&] { return isActive(); } );
}

