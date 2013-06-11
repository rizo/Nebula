#pragma once

#include "../Computer.hpp"
#include "../Simulation.hpp"
#include "../Sdl.hpp"
#include "MonitorFont.hpp"

#include <array>
#include <chrono>

namespace nebula {

namespace sim {

const int MONITOR_PIXELS_PER_DOT = 5;

const int MONITOR_PIXELS_PER_BORDER = 10;

const int MONITOR_DOTS_PER_SCREEN_WIDTH = 128;
const int MONITOR_DOTS_PER_SCREEN_HEIGHT = 96;

const int MONITOR_DOTS_PER_CELL_WIDTH = 4;
const int MONITOR_DOTS_PER_CELL_HEIGHT = 8;

const int MONITOR_CELLS_PER_SCREEN_WIDTH = 32;
const int MONITOR_CELLS_PER_SCREEN_HEIGHT = 12;
const int MONITOR_CELLS_PER_SCREEN =
    MONITOR_CELLS_PER_SCREEN_WIDTH * MONITOR_CELLS_PER_SCREEN_HEIGHT;

const int MONITOR_PIXELS_PER_SCREEN_WIDTH =
    (2 * MONITOR_PIXELS_PER_BORDER)
    + (MONITOR_DOTS_PER_SCREEN_WIDTH * MONITOR_PIXELS_PER_DOT);

const int MONITOR_PIXELS_PER_SCREEN_HEIGHT =
    (2 * MONITOR_PIXELS_PER_BORDER)
    + (MONITOR_DOTS_PER_SCREEN_HEIGHT * MONITOR_PIXELS_PER_DOT);

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

const std::chrono::microseconds MONITOR_FRAME_DURATION { 16666 };

const std::chrono::milliseconds MONITOR_BLINK_DURATION { 500 };

const std::chrono::milliseconds MONITOR_START_UP_DURATION { 1250 };

}

enum class MonitorOperation {
    MapVideoMemory,
    MapFontMemory,
    MapPaletteMemory,
    SetBorderColor,
    DumpFont,
    DumpPalette
};

SAFE_VALUE_WRAPPER( Character, std::uint8_t );
SAFE_VALUE_WRAPPER( BackgroundColor, std::uint8_t );
SAFE_VALUE_WRAPPER( ForegroundColor, std::uint8_t );
SAFE_VALUE_WRAPPER( BorderColor, std::uint8_t );

struct MonitorState {
    optional<std::chrono::microseconds> timeSinceConnected {};
    bool isConnected { false };
    Word videoOffset { 0 };
    Word fontOffset { 0 };
    Word paletteOffset { 0 };
    BorderColor borderColor { 9 };
    
    std::array<bool, sim::MONITOR_CELLS_PER_SCREEN> isBlinking;
    bool blinkVisible { true };
    std::chrono::microseconds sinceLastBlink { 0 };
};

class Monitor : public Simulation<MonitorState>, public Device {
    Computer& _computer;
    std::shared_ptr<ProcessorInterrupt> _procInt { nullptr };
    MonitorState _state;
    std::shared_ptr<Memory> _memory { nullptr };

    sdl::UniqueSurface _screen { nullptr };
    
    std::unique_ptr<SDL_Rect> _dot;
    std::unique_ptr<SDL_Rect> _borderHorizontal;
    std::unique_ptr<SDL_Rect> _borderVertical;

    std::pair<Word, Word> getCharacter( Character ch );

    DoubleWord getColor( std::uint8_t color ) const;
    inline DoubleWord getColor( ForegroundColor color ) const { return getColor( color.value ); }
    inline DoubleWord getColor( BackgroundColor color ) const { return getColor( color.value ); }
    inline DoubleWord getColor( BorderColor color ) const { return getColor( color.value ); }

    void drawBorder();
    void drawFromMemory();
    void drawCell( int x, int y, Character ch, ForegroundColor fg, BackgroundColor bg );
    void drawStartUp();

    void fill( BackgroundColor bg );
    inline void clear() { fill( BackgroundColor { 0 } ); }
    inline void update() { SDL_Flip( _screen.get() ); }

    void handleInterrupt( MonitorOperation op, ProcessorState* proc );
public:
    explicit Monitor( Computer& computer );

    virtual std::unique_ptr<MonitorState> run() override;

    inline virtual DeviceInfo info() const noexcept override {
        return {
            device::Id { 0x7349f615 },
            device::Manufacturer { 0x1c6c8b36 },
            device::Version { 0x1802 }
        };
    }
};

}
