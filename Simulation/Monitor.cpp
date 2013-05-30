#include "Monitor.hpp"

std::unique_ptr<MonitorState> Monitor::run() {
    LOG( INFO ) << "Monitor simulation is active.";
    
    _screen = SDL_SetVideoMode( sim::MONITOR_PIXEL_WIDTH,
                                sim::MONITOR_PIXEL_HEIGHT,
                                16,
                                SDL_SWSURFACE );

    for ( int i = 0; i < 16; ++i ) {
        drawCell( i, 0, i );
    }

    update();

    while ( isActive() ) {
        SDL_Delay( 10 );
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

void Monitor::drawCell( int x, int y, int colorOffset ) {
    _cell->x = x * sim::MONITOR_CELL_WIDTH;
    _cell->y = y * sim::MONITOR_CELL_HEIGHT;

    auto color = mapColor( sim::MONITOR_DEFAULT_PALETTE[colorOffset] );

    SDL_FillRect( _screen, _cell.get(), color );
}
