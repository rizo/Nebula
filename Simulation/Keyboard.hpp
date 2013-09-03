// Simulation/Keyboard.hpp
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

#include "../Computer.hpp"
#include "../Simulation.hpp"
#include "../Sdl.hpp"

#include <atomic>

DEFINE_LOGGER( KEYBOARD, "Keyboard" )

namespace nebula {

namespace sim {

const std::chrono::milliseconds KEYBOARD_SLEEP_DURATION { 5 };

}

enum class KeyboardOperation {
    Clear,
    Store,
    Query,
    EnableInterrupts,
};

class KeyboardState {
    std::atomic<Word> _key;
    std::atomic<bool> _hasKey;
public:
    explicit KeyboardState() :
        _key { 0 },
        _hasKey { false } {}

    explicit KeyboardState( const KeyboardState& other ) :
        _key { other._key.load() },
        _hasKey { other._hasKey.load() } {}

    bool interruptSent { false };
    bool interruptsEnabled { false };
    Word message { 0 };

    inline bool hasKey() const noexcept { return _hasKey.load(); }

    void setKey( const SDL_keysym* ks ) noexcept;

    inline optional<Word> key() const noexcept {
        if ( hasKey() ) {
            return _key.load();
        } else {
            return {};
        }
    }

    inline void clear() noexcept { _hasKey.store( false ); }
};

class Keyboard : public Simulation<KeyboardState>, public Device {
    Computer& _computer;
    std::shared_ptr<ProcessorInterrupt> _procInt { nullptr };
    KeyboardState _state {};

    void handleInterrupt( KeyboardOperation op, ProcessorState* proc );
public:
    explicit Keyboard( Computer& computer ) :
        Simulation<KeyboardState> {},
        _computer( computer ),
        _procInt { computer.nextInterrupt( this ) } {}

    virtual std::unique_ptr<KeyboardState> run() override;

    inline KeyboardState& state() noexcept { return _state; }

    virtual DeviceInfo info() const noexcept override {
        return DeviceInfo {
            device::Id { 0x30cf7406 },
            device::Manufacturer { 0 },
            device::Version { 1 }
        };
    }
};

}
