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

void InterruptQueue::push( Word message ) {
    if ( _queuingEnabled ) {
        if ( _q.size() >= computer::MAX_QUEUED_INTERRUPTS ) {
            throw error::CaughtFire {};
        }

        std::lock_guard<std::mutex> _lock { _mutex };
        _q.emplace( message );
        _isReady.store( true );
    }
}

Word InterruptQueue::pop() {
    std::lock_guard<std::mutex> _lock { _mutex };
    auto res = _q.front();
    _q.pop();

    if ( _q.empty() ) {
        _isReady.store( false );
    }

    return res;
}

