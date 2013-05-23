#pragma once

#include "ProcessorState.hpp"

#include <array>
#include <atomic>
#include <condition_variable>
#include <stdexcept>

using DeviceId = std::uint32_t;

class Device {
public:
    virtual DeviceId id() const = 0;
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

    ProcessorState& processor() { return *_proc; }
    bool isActive() { return _isActive.load(); }

    void respond();
    void waitForTrigger();
};

namespace computer {

constexpr int MAX_DEVICES = 0x10000;

}

namespace error {

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
    std::array<DeviceId, computer::MAX_DEVICES> _devIds {};
    Word _devIndex { 0 };

    std::shared_ptr<Memory> _memory = nullptr;
public:
    explicit Computer( std::shared_ptr<Memory> memory ) :
        _memory { memory } {}

    std::shared_ptr<Memory> memory() { return _memory; }

    inline std::shared_ptr<ProcessorInterrupt> nextInterrupt( const Device& dev ) {
        if ( static_cast<int>( _devIndex ) >= computer::MAX_DEVICES ) {
            throw error::TooManyDevices {};
        }

        _procInts[_devIndex] = std::make_shared<ProcessorInterrupt>();
        _devIds[_devIndex] = dev.id();

        return _procInts[_devIndex++];
    }

    inline std::shared_ptr<ProcessorInterrupt> interruptByIndex( Word index ) {
        if ( index >= _devIndex ) {
            throw error::NoSuchDeviceIndex { index };
        }

        return _procInts[index];
    }

    inline DeviceId idByIndex( Word index ) {
        if ( index >= _devIndex ) {
            throw error::NoSuchDeviceIndex { index };
        }

        return _devIds[index];
    }
};
