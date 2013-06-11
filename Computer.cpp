#include "Computer.hpp"

namespace nebula {

void ProcessorInterrupt::trigger( std::unique_ptr<ProcessorState> proc ) {
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
    if ( _isEnabled ) {
        if ( _q.size() >= computer::MAX_QUEUED_INTERRUPTS ) {
            throw error::CaughtFire {};
        }

        std::lock_guard<std::mutex> _lock { _mutex };
        _q.emplace( message );
        _hasInterrupt.store( true );
    }
}

Word InterruptQueue::pop() {
    std::lock_guard<std::mutex> _lock { _mutex };
    auto res = _q.front();
    _q.pop();

    if ( _q.empty() ) {
        _hasInterrupt.store( false );
    }

    return res;
}

std::shared_ptr<ProcessorInterrupt>
Computer::nextInterrupt( const Device* const  dev ) {
    if ( _devIndex >= computer::MAX_DEVICES ) {
        throw error::TooManyDevices {};
    }

    _procInts[_devIndex] = std::make_shared<ProcessorInterrupt>();
    _devInfo[_devIndex] = dev->info();

    return _procInts[_devIndex++];
}

std::shared_ptr<ProcessorInterrupt>
Computer::interruptByIndex( std::size_t index ) {
    if ( index >= _devIndex ) {
        throw error::NoSuchDeviceIndex { index };
    }

    return _procInts[index];
}

DeviceInfo Computer::infoByIndex( std::size_t index ) {
    if ( index >= _devIndex ) {
        throw error::NoSuchDeviceIndex { index };
    }

    return *_devInfo[index];
}

}
