/**
   \file sdl.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   \brief SDL wrappers common to multiple SDL sub-systems.
 */

#ifndef NEBULA_SDL_HPP_
#define NEBULA_SDL_HPP_

#pragma once

extern "C" {
#include <SDL2/SDL.h>
}

namespace nebula {

namespace error {

class SdlError : public std::runtime_error {
 public:
  explicit SdlError(const std::string& message) : std::runtime_error{message} {}
};

}  // namespace error

inline void wrapSdl(int result, const std::string& message) {
  if (result != 0) {
    throw error::SdlError{(format("%s: %s.") % message % SDL_GetError()).str()};
  }
}

}  // namespace nebula

#endif  // NEBULA_SDL_HPP_
