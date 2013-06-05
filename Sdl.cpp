#include "Sdl.hpp"
#include "Simulation/Monitor.hpp"

namespace sdl {

UniqueSurface SCREEN { nullptr };

void initialize() {
    SDL_Init( SDL_INIT_VIDEO );
    std::atexit( &SDL_Quit );

    SCREEN.reset ( SDL_SetVideoMode( sim::MONITOR_PIXELS_PER_SCREEN_WIDTH,
                                     sim::MONITOR_PIXELS_PER_SCREEN_HEIGHT,
                                     16,
                                     SDL_HWSURFACE | SDL_DOUBLEBUF ) );
}

}
