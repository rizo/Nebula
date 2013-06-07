#include "Sdl.hpp"
#include "Simulation/Monitor.hpp"

namespace nebula {

namespace sdl {

UniqueSurface SCREEN { nullptr };

void initialize() {
    SDL_Init( SDL_INIT_VIDEO );
    std::atexit( &SDL_Quit );

    SDL_EnableUNICODE( 1 );
    SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );

    SCREEN.reset ( SDL_SetVideoMode( sim::MONITOR_PIXELS_PER_SCREEN_WIDTH,
                                     sim::MONITOR_PIXELS_PER_SCREEN_HEIGHT,
                                     16,
                                     SDL_HWSURFACE | SDL_DOUBLEBUF ) );
}

}

}
