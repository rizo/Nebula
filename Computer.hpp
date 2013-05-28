#pragma once

#include "ProcessorState.hpp"

#include <array>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <stdexcept>

namespace computer {

constexpr int MAX_DEVICES = 0x10000;
constexpr int MAX_QUEUED_INTERRUPTS = 256;

}

namespace device {

struct Id { DoubleWord value; };
struct Manufacturer { DoubleWord value; };
struct Version { Word value; };

}

struct DeviceInfo {
    device::Id id;
    device::Manufacturer manufacturer;
    device::Version version;
};

class Device {
public:
    virtual DeviceInfo info() const = 0;
};

class ProcessorInterrupt {
    std::atomic<bool> _isActive { false };
    std::condition_variable _condition;
    std::mutex _mutex;
    std::unique_ptr<ProcessorState> _proc = nullptr;
public:
    // Sender interface.

    void trigger( std::unique_ptr<ProcessorState>&& proc );
    std::unique_ptr<ProcessorState> waitForResponse();

    // Receiver interface.

    ProcessorState* state() { return _proc.get(); }
    bool isActive() { return _isActive.load(); }

    void respond();
    void waitForTrigger();
};

class InterruptQueue {
    std::queue<Word> _q {};
    std::mutex _mutex {};
    std::atomic<bool> _queuingEnabled { false };

    // Used so that checking the queue doesn't require locking it.
    std::atomic<bool> _isReady { false };
public:
    void push( Word message );
    Word pop();
    bool isReady() const { return _isReady.load(); }

    void enable() { _queuingEnabled.store( true ); }
    void disable() { _queuingEnabled.store( false ); }
};

namespace error {

class CaughtFire : public std::runtime_error {
public:
    explicit CaughtFire() :
        std::runtime_error { "DCPU-16 caught fire!" } {}
};

class TooManyDevices : public std::out_of_range {
public:
    explicit TooManyDevices() :
        std::out_of_range {
            (format( "Cannot exceed maximum of %d attached devices" ) % computer::MAX_DEVICES ).str()
        } {}
};

class NoSuchDeviceIndex : public std::out_of_range {
    Word _index;
public:
    explicit NoSuchDeviceIndex( Word index ) :
        std::out_of_range {
            (format( "No such device index: 0x%04x" ) % index).str()
        },
        _index { index } {}

    Word index() const { return _index; }
};

}

class Computer {
    std::array<std::shared_ptr<ProcessorInterrupt>, computer::MAX_DEVICES> _procInts {};
    std::array<DeviceInfo, computer::MAX_DEVICES> _devInfo {};
    Word _devIndex { 0 };

    InterruptQueue _intQ {};
    bool _onlyQueuing { false };

    Word _ia { 0 };

    std::shared_ptr<Memory> _memory = nullptr;
public:
    explicit Computer( std::shared_ptr<Memory> memory ) :
        _memory { memory } {}

    std::shared_ptr<Memory> memory() { return _memory; }

    inline std::shared_ptr<ProcessorInterrupt> nextInterrupt( const Device* const  dev ) {
        if ( static_cast<int>( _devIndex ) >= computer::MAX_DEVICES ) {
            throw error::TooManyDevices {};
        }

        _procInts[_devIndex] = std::make_shared<ProcessorInterrupt>();
        _devInfo[_devIndex] = dev->info();

        return _procInts[_devIndex++];
    }

    inline std::shared_ptr<ProcessorInterrupt> interruptByIndex( Word index ) {
        if ( index >= _devIndex ) {
            throw error::NoSuchDeviceIndex { index };
        }

        return _procInts[index];
    }

    inline DeviceInfo infoByIndex( Word index ) {
        if ( index >= _devIndex ) {
            throw error::NoSuchDeviceIndex { index };
        }

        return _devInfo[index];
    }

    inline int numDevices() const { return _devIndex; }
    inline InterruptQueue& queue() { return _intQ; }

    inline void setOnlyQueueing( bool val ) { _onlyQueuing = val; }

    inline Word ia() const { return _ia; }
    inline void setIa( Word location ) { _ia = location; }
};
