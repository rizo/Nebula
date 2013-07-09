// Sdl.cpp
//
// Copyright 2013 Jesse Haber-Kucharsky
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
