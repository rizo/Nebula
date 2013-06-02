#include "Monitor.hpp"

std::unique_ptr<MonitorState> Monitor::run() {
    setActive();

    LOG( INFO ) << "Monitor simulation is active.";
    
    _screen = SDL_SetVideoMode( sim::MONITOR_PIXELS_PER_SCREEN_WIDTH,
                                sim::MONITOR_PIXELS_PER_SCREEN_HEIGHT,
                                16,
                                SDL_SWSURFACE );

    while ( isActive() )  {
        if ( ! _state.isConnected ) {
            LOG( INFO ) << "Monitor is disconnected. Waiting for interrupt.";

            _procInt->waitForTriggerOrDeath( *this );
        }

        if ( _procInt->isActive() ) {
            LOG( INFO ) << "Monitor got interrupt.";

            auto proc = _procInt->state();
            auto a = proc->read( Register::A );

            switch ( a ) {
            case 0:
                handleInterrupt( MonitorOperation::MapVideoMemory, proc );
                break;
            case 3:
                handleInterrupt( MonitorOperation::SetBorderColor, proc );
                break;
            }

            _procInt->respond();

            LOG( INFO ) << "Monitor handled interrupt.";
        }

        if ( !_state.isConnected ) {
            clear();
            update();
        } else {
            clear();
            drawBorder();
            drawFromMemory();
            update();   
        }

        std::this_thread::sleep_for( sim::MONITOR_FRAME_DURATION );

        _state.sinceLastBlink += sim::MONITOR_FRAME_DURATION;

        if ( _state.sinceLastBlink >= std::chrono::duration_cast<std::chrono::microseconds>( sim::MONITOR_BLINK_DURATION ) ) {
            _state.sinceLastBlink = std::chrono::microseconds { 0 };
            _state.blinkVisible = ! _state.blinkVisible;
        }
    }

    LOG( INFO ) << "Monitor simulation shutting down.";
    return {};
}

void Monitor::drawBorder() {
    auto color = mapColor( getColor( _state.borderColor ) );

    // Top.
    _borderHorizontal->x = 0;
    _borderHorizontal->y = 0;
    SDL_FillRect( _screen, _borderHorizontal.get(), color );

    // Bottom.
    _borderHorizontal->x = 0;
    _borderHorizontal->y = sim::MONITOR_PIXELS_PER_SCREEN_HEIGHT - sim::MONITOR_PIXELS_PER_BORDER;
    SDL_FillRect( _screen, _borderHorizontal.get(), color );

    // Left.
    _borderVertical->x = 0;
    _borderVertical->y = 0;
    SDL_FillRect( _screen, _borderVertical.get(), color );

    // Right.
    _borderVertical->x = sim::MONITOR_PIXELS_PER_SCREEN_WIDTH - sim::MONITOR_PIXELS_PER_BORDER;
    _borderVertical->y = 0;
    SDL_FillRect( _screen, _borderVertical.get(), color );
}

static inline int index( int x, int y ) {
    return (y * sim::MONITOR_CELLS_PER_SCREEN_WIDTH) + x;
}

void Monitor::drawFromMemory() {
    Word w;

    for ( int y = 0; y < sim::MONITOR_CELLS_PER_SCREEN_HEIGHT; ++y ) {
        for ( int x = 0; x < sim::MONITOR_CELLS_PER_SCREEN_WIDTH; ++x ) {
            auto loc = (y * sim::MONITOR_CELLS_PER_SCREEN_WIDTH) + x + _state.videoOffset;
            w = _memory->read( loc );

            bool doBlink = static_cast<bool>( w & 0x0080 );
            auto ch = static_cast<std::uint8_t>( w & 0x007f );
            auto fg = static_cast<std::uint8_t>( (w & 0xf000) >> 12 );
            auto bg = static_cast<std::uint8_t>( (w & 0x0f00) >> 8 );

            if ( doBlink ) {
                _state.isBlinking[index( x, y )] = true;
            }

            drawCell( x, y,
                      Character { ch },
                      ForegroundColor { fg },
                      BackgroundColor { bg } );
        }
    }
}

void Monitor::handleInterrupt( MonitorOperation op, ProcessorState* proc ) {
    Word b = proc->read( Register::B );

    switch ( op ) {
    case MonitorOperation::SetBorderColor:
        LOG( INFO ) << "Monitor executing 'SetBorderColor'.";

        _state.borderColor = BorderColor { static_cast<std::uint8_t>( b & 0xf ) };
        break;
    case MonitorOperation::MapVideoMemory:
        LOG( INFO ) << "Monitor executing 'MapVideoMemory'.";

        if ( b != 0 ) {
            LOG( INFO ) << format( "Connecting monitor with video memory at 0x%04x." ) % b;

            _state.isConnected = true;
            _state.videoOffset = b;
        } else {
            LOG( WARNING ) << "Disconnecting monitor.";
            _state.isConnected = false;
        }

        break;
    }
}

DoubleWord Monitor::mapColor( Word color ) {
    Word bluePart = color & 0x000f;
    Word greenPart = (color & 0x00f0) >> 4;
    Word redPart = (color & 0x0f00) >> 8;

    Word blue = bluePart | (bluePart << 4);
    Word green = greenPart | (greenPart << 4);
    Word red = redPart | (redPart << 4);

    return SDL_MapRGB( _screen->format, red, green, blue );
}

void Monitor::clear() {
    auto black = SDL_MapRGB( _screen->format, 0, 0, 0 );
    SDL_FillRect( _screen, nullptr, black );
}

Word Monitor::getColor( std::uint8_t color ) {
    if ( _state.paletteOffset == 0 ) {
        return sim::MONITOR_DEFAULT_PALETTE[color];
    } else {
        return _memory->read( _state.paletteOffset + color );
    }
}


void Monitor::drawCell( int x, int y,
                        Character ch,
                        ForegroundColor fg,
                        BackgroundColor bg ) {
    int cellX = x * sim::MONITOR_PIXELS_PER_CELL_WIDTH;
    int cellY = y * sim::MONITOR_PIXELS_PER_CELL_HEIGHT;

    auto fgColor = mapColor( getColor( fg ) );
    auto bgColor = mapColor( getColor( bg ) );

    auto drawDot = [&] ( int x, int y, bool isForeground) {
        _dot->x = sim::MONITOR_PIXELS_PER_BORDER + cellX + (x * sim::MONITOR_PIXELS_PER_DOT_WIDTH );
        _dot->y = sim::MONITOR_PIXELS_PER_BORDER + cellY + (y * sim::MONITOR_PIXELS_PER_DOT_HEIGHT );

        auto color = isForeground ? fgColor : bgColor;
        SDL_FillRect( _screen, _dot.get(), color );
    };

    bool isBlinking = _state.isBlinking[index( x, y )];

    auto drawColumn = [&] ( std::uint8_t columnData, int x ) {
        for ( std::uint8_t i = 0; i < sim::MONITOR_DOTS_PER_CELL_HEIGHT; ++i ) {
            bool drawFg = (columnData & (1 << i)) &&
                          ((isBlinking && _state.blinkVisible) || ! isBlinking);
            drawDot( x, i, drawFg );
        }
    };

    auto data = sim::MONITOR_DEFAULT_FONT[ch.value];
    Word first = data.first;
    Word second = data.second;

    drawColumn( (first & 0xff00) >> 8, 0 );
    drawColumn( first & 0x00ff, 1 );
    drawColumn( (second & 0xff00) >> 8, 2 );
    drawColumn( second & 0x00ff, 3 );
}
