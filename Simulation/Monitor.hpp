#pragma once

#include "../Computer.hpp"
#include "../Simulation.hpp"

#include <array>
#include <chrono>

#include <SDL/SDL.h>

namespace sim {

const int MONITOR_PIXEL_MULTIPLIER = 5;

const int MONITOR_PIXEL_WIDTH = 128 * MONITOR_PIXEL_MULTIPLIER;
const int MONITOR_PIXEL_HEIGHT = 96 * MONITOR_PIXEL_MULTIPLIER;

const int MONITOR_CELL_WIDTH = 4 * MONITOR_PIXEL_MULTIPLIER;
const int MONITOR_CELL_HEIGHT = 8 * MONITOR_PIXEL_MULTIPLIER;

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

constexpr std::chrono::microseconds MONITOR_FRAME_DURATION { 16666 };

}

struct BackgroundColor { Word value; };
struct ForegroundColor { Word value; };

struct MonitorState {
    Word videoOffset { 0 };
    Word fontOffset { 0 };
    Word paletteOffset { 0 };
    Word borderColor { 0 };
};

class Monitor : public Simulation<MonitorState>, public Device {
    SDL_Surface* _screen { nullptr };
    std::unique_ptr<SDL_Rect> _cell;

    void drawCell( int x, int y, int colorOffset );
    DoubleWord mapColor( Word color );
    void update() { SDL_Flip( _screen ); }
public:
    explicit Monitor() :
        _cell { make_unique<SDL_Rect>() } {
        _cell->x = 0;
        _cell->y = 0;
        _cell->w = sim::MONITOR_CELL_WIDTH;
        _cell->h = sim::MONITOR_CELL_HEIGHT;
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
