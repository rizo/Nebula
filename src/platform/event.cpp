/**
   \file event.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   Events are an abstraction over SDL events, but more type-safe.
 */

#include "event.hpp"

extern "C" {
#include <SDL2/SDL.h>
}

#include <cstring>

namespace nebula {

namespace event {

optional<Event> poll() {
  SDL_Event event;
  char ch;

  if (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        return Event{Quit{}};
      case SDL_KEYDOWN:
        // Map certain characters to the code expected by the virtual keyboard.
        switch (event.key.keysym.sym) {
          case SDLK_BACKSPACE:
            return Event{KeyInput{0x10}};

          case SDLK_RETURN:
            return Event{KeyInput{0x11}};

          case SDLK_INSERT:
            return Event{KeyInput{0x12}};

          case SDLK_DELETE:
            return Event{KeyInput{0x13}};

          case SDLK_UP:
            return Event{KeyInput{0x80}};

          case SDLK_DOWN:
            return Event{KeyInput{0x81}};

          case SDLK_LEFT:
            return Event{KeyInput{0x82}};

          case SDLK_RIGHT:
            return Event{KeyInput{0x83}};

          case SDLK_LSHIFT:
          case SDLK_RSHIFT:
            return Event{KeyInput{0x90}};

          case SDLK_LCTRL:
          case SDLK_RCTRL:
            return Event{KeyInput{0x91}};

          default:
            return {};
        }

      case SDL_TEXTINPUT:
        if (std::strlen(event.text.text) == 1u) {
          ch = *event.text.text;

          if (ch >= 0x20) {
            return Event{KeyInput{static_cast<Word>(ch)}};
          }
        }

      default:
        // An event we don't wish to recognize.
        return {};
    }
  }

  return {};
}

}  // namespace event

}  // namespace nebula
