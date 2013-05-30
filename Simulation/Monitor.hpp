#pragma once

#include "../Computer.hpp"
#include "../Simulation.hpp"

#include <array>
#include <chrono>

#include <boost/utility.hpp>
#include <SDL/SDL.h>

#define B BOOST_BINARY

namespace sim {

const int MONITOR_PIXELS_PER_DOT = 5;

const int MONITOR_DOTS_PER_SCREEN_WIDTH = 128;
const int MONITOR_DOTS_PER_SCREEN_HEIGHT = 96;

const int MONITOR_DOTS_PER_CELL_WIDTH = 4;
const int MONITOR_DOTS_PER_CELL_HEIGHT = 8;

const int MONITOR_PIXELS_PER_SCREEN_WIDTH = MONITOR_DOTS_PER_SCREEN_WIDTH * MONITOR_PIXELS_PER_DOT;
const int MONITOR_PIXELS_PER_SCREEN_HEIGHT = MONITOR_DOTS_PER_SCREEN_HEIGHT * MONITOR_PIXELS_PER_DOT;

const int MONITOR_PIXELS_PER_CELL_WIDTH = MONITOR_DOTS_PER_CELL_WIDTH * MONITOR_PIXELS_PER_DOT;
const int MONITOR_PIXELS_PER_CELL_HEIGHT = MONITOR_DOTS_PER_CELL_HEIGHT * MONITOR_PIXELS_PER_DOT;

const int MONITOR_PIXELS_PER_DOT_WIDTH = MONITOR_PIXELS_PER_CELL_WIDTH / MONITOR_DOTS_PER_CELL_WIDTH;
const int MONITOR_PIXELS_PER_DOT_HEIGHT = MONITOR_PIXELS_PER_CELL_HEIGHT / MONITOR_DOTS_PER_CELL_HEIGHT;

const std::array<Word, 16> MONITOR_DEFAULT_PALETTE { {
    0x0000,
    0x000a,
    0x00a0,
    0x00aa,
    0x0a00,
    0x0a0a,
    0x0a50,
    0x0aaa,
    0x0555,
    0x055f,
    0x05f5,
    0x05ff,
    0x0f55,
    0x0f5f,
    0x0ff5,
    0x0fff
} };

const std::array<std::pair<Word, Word>, 1> MONITOR_DEFAULT_FONT { {
        { B( 11111110
             10010010 ),
          B( 10000010
             00000000 ) }
} };


constexpr std::chrono::microseconds MONITOR_FRAME_DURATION { 16666 };

}

struct Character { std::uint8_t value; };
struct BackgroundColor { std::uint8_t value; };
struct ForegroundColor { std::uint8_t value; };

struct MonitorState {
    Word videoOffset { 0 };
    Word fontOffset { 0 };
    Word paletteOffset { 0 };
    Word borderColor { 0 };
};

class Monitor : public Simulation<MonitorState>, public Device {
    SDL_Surface* _screen { nullptr };
    std::unique_ptr<SDL_Rect> _dot;

    void drawCell( int x, int y, Character ch, ForegroundColor fg, BackgroundColor bg );
    DoubleWord mapColor( Word color );
    void update() { SDL_Flip( _screen ); }
public:
    explicit Monitor() :
        _dot { make_unique<SDL_Rect>() } {
        _dot->x = 0;
        _dot->y = 0;
        _dot->w = sim::MONITOR_PIXELS_PER_DOT_WIDTH;
        _dot->h = sim::MONITOR_PIXELS_PER_DOT_HEIGHT;
    }

    virtual std::unique_ptr<MonitorState> run();

    virtual DeviceInfo info() const {
        return {
            device::Id { 0x7349f615 },
            device::Manufacturer { 0x1c6c8b36 },
            device::Version { 0x1802 }
        };
    }

    ~Monitor() {
        if ( _screen ) {
            SDL_FreeSurface( _screen );
        }
    }
};
