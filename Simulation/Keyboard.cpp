#include "Keyboard.hpp"

namespace nebula {

void KeyboardState::setKey( const SDL_keysym* ks ) noexcept {
    Word k { 0 };

    // Check for a letter.
    if ( ks->unicode < 128 && ks->unicode >= 32 ) {
        k = ks->unicode & 0xff;
    } else {
        switch ( ks->sym ) {
        case SDLK_BACKSPACE:
            k = 0x10;
            break;
        case SDLK_RETURN:
            k = 0x11;
            break;
        case SDLK_INSERT:
            k = 0x12;
            break;
        case SDLK_DELETE:
            k = 0x13;
            break;
        case SDLK_UP:
            k = 0x80;
            break;
        case SDLK_DOWN:
            k = 0x81;
            break;
        case SDLK_LEFT:
            k = 0x82;
            break;
        case SDLK_RIGHT:
            k = 0x83;
            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            k = 0x90;
            break;
        case SDLK_LCTRL:
        case SDLK_RCTRL:
            k = 0x91;
            break;
        default:
            break;
        }
    }

    if ( k != 0 ) {
        LOG( KEYBOARD, info ) << format( "Got <%d>" ) % k;

        _key.store( k );
        _hasKey.store( true );
        _interruptSent = false;
    }
}

std::unique_ptr<KeyboardState> Keyboard::run() {
    setActive();

    LOG( KEYBOARD, info ) << "Simulation is active.";

    while ( isActive() ) {
        if ( _state.hasKey() &&
             _state.interruptsEnabled() &&
             ! _state.interruptSent() ) {

            _computer.queue().push( _state.message() );
            _state.setInterruptSent( true );
        }

        if ( _procInt->isActive() ) {
            LOG( KEYBOARD, info ) << "Got interrupt.";

            auto proc = _procInt->state();
            auto a = proc->read( Register::A );

            switch ( a ) {
            case 0:
                handleInterrupt( KeyboardOperation::Clear, proc );
                break;
            case 1:
                handleInterrupt( KeyboardOperation::Store, proc );
                break;
            case 2:
                handleInterrupt( KeyboardOperation::Query, proc );
                break;
            case 3:
                handleInterrupt( KeyboardOperation::EnableInterrupts, proc );
                break;
            }

            _procInt->respond();

            LOG( KEYBOARD, info ) << "Handled interrupt.";
        }

        std::this_thread::sleep_for( sim::KEYBOARD_SLEEP_DURATION );
    }

    LOG( KEYBOARD, info ) << "Shutting down.";
    return make_unique<KeyboardState>( _state );
}

void Keyboard::handleInterrupt( KeyboardOperation op, ProcessorState* proc ) {
    Word b;

    switch ( op ) {
    case KeyboardOperation::Clear:
        LOG( KEYBOARD, info ) << "'Clear'";
        _state.clear();

        break;
    case KeyboardOperation::Store:
        LOG( KEYBOARD, info ) << "'Store'";

        if ( ! _state.hasKey() ) {
            proc->write( Register::C, 0 );
        } else {
            proc->write( Register::C, *_state.key() );
        }

        break;
    case KeyboardOperation::Query:
        LOG( KEYBOARD, info ) <<  "'Query'";

        b = proc->read( Register::B );
        if ( _state.hasKey() ) {
            proc->write( Register::C, *_state.key() == b );
        } else {
            proc->write( Register::C, 0 );
        }

        break;
    case KeyboardOperation::EnableInterrupts:
        LOG( KEYBOARD, info ) << "'EnableInterrupts'";

        b = proc->read( Register::B );

        if ( b != 0 ) {
            _state.setInterruptsEnabled( true );
            _state.setMessage( b );
        } else {
            _state.setInterruptsEnabled( false );
        }

        break;
    }
}

}
