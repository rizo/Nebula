#include "../Sdl.hpp"
#include "Monitor.hpp"

namespace nebula {

Monitor::Monitor( Computer& computer ) :
    Simulation<MonitorState> {},
    _computer( computer ),
    _procInt { computer.nextInterrupt( this ) },
    _memory { computer.memory() },
    _dot { make_unique<SDL_Rect>() },
    _borderHorizontal { make_unique<SDL_Rect>() },
    _borderVertical { make_unique<SDL_Rect>() } {

    _state.isBlinking.fill( false );

    _borderHorizontal->w = sim::MONITOR_PIXELS_PER_SCREEN_WIDTH;
    _borderHorizontal->h = sim::MONITOR_PIXELS_PER_BORDER;

    _borderVertical->w = sim::MONITOR_PIXELS_PER_BORDER;
    _borderVertical->h = sim::MONITOR_PIXELS_PER_SCREEN_HEIGHT;

    _dot->x = 0;
    _dot->y = 0;
    _dot->w = sim::MONITOR_PIXELS_PER_DOT_WIDTH;
    _dot->h = sim::MONITOR_PIXELS_PER_DOT_HEIGHT;
}

std::unique_ptr<MonitorState> Monitor::run() {
    setActive();

    LOG( INFO ) << "Monitor simulation is active.";
    _screen = std::move( sdl::SCREEN );

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
            case 1:
                handleInterrupt( MonitorOperation::MapFontMemory, proc );
                break;
            case 2:
                handleInterrupt( MonitorOperation::MapPaletteMemory, proc );
                break;
            case 3:
                handleInterrupt( MonitorOperation::SetBorderColor, proc );
                break;
            case 4:
                handleInterrupt( MonitorOperation::DumpFont, proc );
                break;
            case 5:
                handleInterrupt( MonitorOperation::DumpPalette, proc );
                break;
            }

            _procInt->respond();

            LOG( INFO ) << "Monitor handled interrupt.";
        }

        clear();

        if ( _state.isConnected ) {
            // Show the start-up image if the monitor is still initializing.
            if ( _state.timeSinceConnected ) {
                *_state.timeSinceConnected += sim::MONITOR_FRAME_DURATION;
                drawStartUp();

                if ( *_state.timeSinceConnected >=
                     std::chrono::duration_cast<std::chrono::microseconds>( sim::MONITOR_START_UP_DURATION ) ) {

                    _state.timeSinceConnected.reset();
                }
            } else {
                drawBorder();
                drawFromMemory();
            }
        }

        update();
        std::this_thread::sleep_for( sim::MONITOR_FRAME_DURATION );

        _state.sinceLastBlink += sim::MONITOR_FRAME_DURATION;

        if ( _state.sinceLastBlink >=
             std::chrono::duration_cast<std::chrono::microseconds>( sim::MONITOR_BLINK_DURATION ) ) {

            _state.sinceLastBlink = std::chrono::microseconds { 0 };
            _state.blinkVisible = ! _state.blinkVisible;
        }
    }

    LOG( INFO ) << "Monitor simulation shutting down.";
    return {};
}

void Monitor::drawStartUp() {
    const int MID_X = (sim::MONITOR_CELLS_PER_SCREEN_WIDTH / 2) - 1;
    const int MID_Y = (sim::MONITOR_CELLS_PER_SCREEN_HEIGHT / 2) - 1;
    const int NUM_LINES = 4;

    fill( BackgroundColor { 0x9 } );

    auto drawCentered = [&] ( int lineIndex, const std::uint8_t* line, std::size_t length ) {
        for ( std::size_t i = 0; i < length; ++i ) {
            drawCell( MID_X - (length / 2) + i,
                      MID_Y - (NUM_LINES / 2) + lineIndex,
                      Character { line[i] },
                      ForegroundColor { 0xe },
                      BackgroundColor { 0x9 } );
        }
    };

    const std::array<std::uint8_t, 14> LINE1 = { 'N', 'Y', 'A', ' ', 'E', 'L', 'E', 'K', 'T', 'R', 'I', 'S', 'K', 'A'};
    const std::array<std::uint8_t, 7 > LINE2 = { 'L', 'E', 'M', '1', '8', '0', '2' };
    const std::array<std::uint8_t, 18> LINE3 = {'L', 'o', 'w', ' ', 'E', 'n', 'e', 'r', 'g', 'y', ' ', 'M', 'o', 'n', 'i', 't', 'o', 'r' };

    drawCentered( 0, LINE1.data(), LINE1.size() );
    drawCentered( 2, LINE2.data(), LINE2.size() );
    drawCentered( 3, LINE3.data(), LINE3.size() );
}

void Monitor::drawBorder() {
    auto color = mapColor( getColor( _state.borderColor ) );

    // Top.
    _borderHorizontal->x = 0;
    _borderHorizontal->y = 0;
    SDL_FillRect( _screen.get(), _borderHorizontal.get(), color );

    // Bottom.
    _borderHorizontal->x = 0;
    _borderHorizontal->y = sim::MONITOR_PIXELS_PER_SCREEN_HEIGHT - sim::MONITOR_PIXELS_PER_BORDER;
    SDL_FillRect( _screen.get(), _borderHorizontal.get(), color );

    // Left.
    _borderVertical->x = 0;
    _borderVertical->y = 0;
    SDL_FillRect( _screen.get(), _borderVertical.get(), color );

    // Right.
    _borderVertical->x = sim::MONITOR_PIXELS_PER_SCREEN_WIDTH - sim::MONITOR_PIXELS_PER_BORDER;
    _borderVertical->y = 0;
    SDL_FillRect( _screen.get(), _borderVertical.get(), color );
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

            bool doBlink = (w & 0x0080) != 0;
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
            _state.timeSinceConnected = std::chrono::microseconds { 0 };
            _state.videoOffset = b;
        } else {
            LOG( WARNING ) << "Disconnecting monitor.";
            _state.isConnected = false;
        }

        break;
    case MonitorOperation::MapPaletteMemory:
        LOG( INFO ) << format( "Monitor executing 'MapPaletteMemory' at 0x%04x" ) % b;

        _state.paletteOffset = b;
        break;
    case MonitorOperation::MapFontMemory:
        LOG( INFO ) << format( "Monitor executing 'MapFontMemory' at 0x%04x" ) % b;

        _state.fontOffset = b;
        break;
    case MonitorOperation::DumpFont:
        LOG( INFO ) << format( "Monitor executing 'DumpFont' at %0x%04x" ) % b;

        for ( Word i = 0; i < 2 * sim::MONITOR_DEFAULT_FONT.size(); i += 2 ) {
            auto letter = sim::MONITOR_DEFAULT_FONT[i];

            _memory->write( b + i, letter.first );
            _memory->write( b + i + 1, letter.second );
        }

        break;
    case MonitorOperation::DumpPalette:
        LOG( INFO ) << format( "Monitor executing 'DumpFont' at %0x%04x" ) % b;

        for ( Word i = 0; i < sim::MONITOR_DEFAULT_PALETTE.size(); ++i ) {
            _memory->write( b + i, sim::MONITOR_DEFAULT_PALETTE[i] );
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

void Monitor::fill( BackgroundColor bg ) {
    auto c = mapColor( getColor( bg ) );
    SDL_FillRect( _screen.get(), nullptr, c );
}

std::pair<Word, Word> Monitor::getCharacter( Character ch ) {
    if ( _state.fontOffset == 0 ) {
        return sim::MONITOR_DEFAULT_FONT[ch.value];
    } else {
        auto offset = _state.fontOffset + ch.value;

        return {
            _memory->read( offset ),
            _memory->read( offset + 1 )
        };
    }
}

Word Monitor::getColor( std::uint8_t color ) const {
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
        SDL_FillRect( _screen.get(), _dot.get(), color );
    };

    bool isBlinking = _state.isBlinking[index( x, y )];

    auto drawColumn = [&] ( std::uint8_t columnData, int x ) {
        for ( std::uint8_t i = 0; i < sim::MONITOR_DOTS_PER_CELL_HEIGHT; ++i ) {
            bool drawFg = (columnData & (1 << i)) &&
                          ((isBlinking && _state.blinkVisible) || ! isBlinking);
            drawDot( x, i, drawFg );
        }
    };

    auto data = getCharacter( ch );
    Word first = data.first;
    Word second = data.second;

    drawColumn( (first & 0xff00) >> 8, 0 );
    drawColumn( first & 0x00ff, 1 );
    drawColumn( (second & 0xff00) >> 8, 2 );
    drawColumn( second & 0x00ff, 3 );
}

}
