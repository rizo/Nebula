// Computer.hpp
//
// Copyright 2013 Jesse Haber-Kucharsky
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "ProcessorState.hpp"
#include "Simulation.hpp"

#include <atomic>
#include <condition_variable>
#include <queue>
#include <stdexcept>
#include <vector>

namespace nebula {

namespace computer {

// We support (0x10000 - 1) devices since an index of zero is used as an
// error value for hardware-related instructions.
const std::size_t MAX_DEVICES = 0xffff;
const std::size_t MAX_QUEUED_INTERRUPTS = 256;

}

namespace device {

SAFE_VALUE_WRAPPER( Id, DoubleWord );
SAFE_VALUE_WRAPPER( Manufacturer, DoubleWord );
SAFE_VALUE_WRAPPER( Version, Word );

}

struct DeviceInfo {
    explicit DeviceInfo( device::Id id,
                         device::Manufacturer manufacturer,
                         device::Version version ) :
        id { id },
        manufacturer { manufacturer },
        version { version } {}
    
    device::Id id;
    device::Manufacturer manufacturer;
    device::Version version;
};

class Device {
public:
    virtual DeviceInfo info() const noexcept = 0;
};

class ProcessorInterrupt final {
    std::atomic<bool> _isActive;
    std::condition_variable _condition {};
    std::mutex _mutex {};
    std::unique_ptr<ProcessorState> _proc = nullptr;
public:
    explicit ProcessorInterrupt() :
        _isActive { false } {}

    explicit ProcessorInterrupt( const ProcessorInterrupt& inter ) :
        _isActive { inter._isActive.load() } {}
    
    // Sender interface.

    void trigger( std::unique_ptr<ProcessorState> proc );
    std::unique_ptr<ProcessorState> waitForResponse();

    // Receiver interface.

    ProcessorState* state() noexcept { return _proc.get(); }
    bool isActive() noexcept { return _isActive.load(); }

    void respond();
    void waitForTrigger();

    template <typename StateType>
    void waitForTriggerOrDeath( Simulation<StateType>& s ) {
        std::unique_lock<std::mutex> lock { _mutex };

        while ( true ) {
            if ( _condition.wait_for( lock, std::chrono::milliseconds { 5 },
                                      [&] { return isActive(); } ) ) {
                // Timeout, and interrupt is active.
                break;
            } else {
                // Check to see if the simulation has died.
                if ( ! s.isActive() ) {
                    break;
                }
            }
        }
    }
};

class InterruptQueue final {
    std::queue<Word> _q {};
    std::mutex _mutex {};

    bool _isEnabled { true };

    // Used so that checking the queue doesn't require locking it.
    std::atomic<bool> _hasInterrupt { false };
public:
    void push( Word message );
    Word pop();
    inline bool hasInterrupt() const noexcept { return _hasInterrupt.load(); }

    inline void setEnabled( bool value ) noexcept { _isEnabled = value; }
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
    std::size_t _index;
public:
    explicit NoSuchDeviceIndex( std::size_t index ) :
        std::out_of_range {
            (format( "No such device index: 0x%04x" ) % index).str()
        },
        _index { index } {}

    std::size_t index() const noexcept { return _index; }
};

}

class Computer final {
    using InterruptTable = std::vector<std::shared_ptr<ProcessorInterrupt>>;
    using DeviceTable = std::vector<optional<DeviceInfo>>;

    InterruptTable _procInts;
    DeviceTable _devInfo;
    std::size_t _devIndex { 1 };

    InterruptQueue _intQ {};
    bool _onlyQueuing { false };

    Word _ia { 0 };

    std::shared_ptr<Memory> _memory { nullptr };
public:
    explicit Computer( std::shared_ptr<Memory> memory ) :
        _procInts { InterruptTable( computer::MAX_DEVICES, nullptr ) },
        _devInfo { DeviceTable( computer::MAX_DEVICES, boost::none ) },
        _memory { memory } {
    }

    inline std::shared_ptr<Memory> memory() noexcept { return _memory; }

    std::shared_ptr<ProcessorInterrupt> nextInterrupt( const Device* const  dev );
    std::shared_ptr<ProcessorInterrupt> interruptByIndex( std::size_t index );
    DeviceInfo infoByIndex( std::size_t index );

    inline std::size_t numDevices() const noexcept { return _devIndex; }

    inline InterruptQueue& queue() noexcept { return _intQ; }

    inline void setOnlyQueuing( bool val ) noexcept { _onlyQueuing = val; }
    inline bool onlyQueuing() const noexcept { return _onlyQueuing; }

    inline Word ia() const noexcept { return _ia; }
    inline void setIa( Word location ) noexcept { _ia = location; }
};

}
