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

    inline bool interruptSent() const noexcept { return _interruptSent; }
    inline void setInterruptSent( bool value ) noexcept { _interruptSent = value; }

    inline bool interruptsEnabled() const noexcept { return _interruptsEnabled; }
    inline void setInterruptsEnabled( bool value ) noexcept { _interruptsEnabled = value; }

    inline Word message() const noexcept { return _message; }
    inline void setMessage( Word value ) noexcept { _message = value; }
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
        return {
            device::Id { 0x30cf7406 },
            device::Manufacturer { 0 },
            device::Version { 1 }
        };
    }
};

}
