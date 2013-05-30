#include "Monitor.hpp"

std::unique_ptr<MonitorState> Monitor::run() {
    setActive();

    LOG( INFO ) << "Monitor simulation is active.";
    
    _screen = SDL_SetVideoMode( sim::MONITOR_PIXELS_PER_SCREEN_WIDTH,
                                sim::MONITOR_PIXELS_PER_SCREEN_HEIGHT,
                                16,
                                SDL_SWSURFACE );

    while ( isActive() )  {
        clear();

        for ( std::uint8_t i = 0; i < 16; ++i ) {
            drawCell( i, 0,
                      Character { 0 },
                      ForegroundColor { i },
                      BackgroundColor { 2 } );
        }

        update();

        std::this_thread::sleep_for( sim::MONITOR_FRAME_DURATION );
    }

    LOG( INFO ) << "Monitor simulation shutting down.";
    return {};
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

void Monitor::drawCell( int x, int y,
                        Character ch,
                        ForegroundColor fg,
                        BackgroundColor bg ) {
    int cellX = x * sim::MONITOR_PIXELS_PER_CELL_WIDTH;
    int cellY = y * sim::MONITOR_PIXELS_PER_CELL_HEIGHT;

    auto fgColor = mapColor( sim::MONITOR_DEFAULT_PALETTE[fg.value] );
    auto bgColor = mapColor( sim::MONITOR_DEFAULT_PALETTE[bg.value] );

    auto drawDot = [&] ( int x, int y, bool isForeground) {
        _dot->x = cellX + (x * sim::MONITOR_PIXELS_PER_DOT_WIDTH );
        _dot->y = cellY + (y * sim::MONITOR_PIXELS_PER_DOT_HEIGHT );

        auto color = isForeground ? fgColor : bgColor;
        SDL_FillRect( _screen, _dot.get(), color );
    };

    auto drawColumn = [&] ( std::uint8_t columnData, int x ) {
        for ( std::uint8_t i = 0; i < sim::MONITOR_DOTS_PER_CELL_HEIGHT; ++i ) {
            drawDot( x, i, columnData & (1 << i) );
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
