#pragma once

#include "../Computer.hpp"
#include "../Simulation.hpp"
#include "../Sdl.hpp"

#include <atomic>

namespace sim {

const std::chrono::milliseconds KEYBOARD_SLEEP_DURATION { 50 };

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
    bool _interruptSent;
    bool _interruptsEnabled;
    Word _message;
public:
    explicit KeyboardState() :
        _key { 0 },
        _hasKey { false },
        _interruptSent { false },
        _interruptsEnabled { false },
        _message { 0 } {}

    explicit KeyboardState( const KeyboardState& other ) :
        _key { other._key.load() },
        _hasKey { other._hasKey.load() } {}

    bool hasKey() const { return _hasKey.load(); }

    optional<Word> key() const {
        if ( hasKey() ) {
            return _key.load();
        } else {
            return {};
        }
    }

    void clear() { _hasKey.store( false ); }

    void setKey( const SDL_keysym* ks );

    bool interruptSent() const { return _interruptSent; }
    void setInterruptSent( bool value ) { _interruptSent = value; }

    bool interruptsEnabled() const { return _interruptsEnabled; }
    void setInterruptsEnabled( bool value ) { _interruptsEnabled = value; }

    Word message() const { return _message; }
    void setMessage( Word value ) { _message = value; }
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

    virtual std::unique_ptr<KeyboardState> run();

    KeyboardState& state() { return _state; }

    virtual DeviceInfo info() const {
        return {
            device::Id { 0x30cf7406 },
            device::Manufacturer { 0 },
            device::Version { 1 }
        };
    }
};

