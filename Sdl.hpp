#pragma once

#include "Fundamental.hpp"

#include <SDL/SDL.h>

namespace nebula {

namespace sdl {

void initialize();

class SurfaceDeleter {
public:
    void operator()( SDL_Surface* s ) {
        if ( s ) {
            SDL_FreeSurface( s );
        }
    }
};

using UniqueSurface = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

extern UniqueSurface SCREEN;

}

}
