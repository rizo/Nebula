find_package (PkgConfig)
pkg_check_modules (PC_SDL2 QUIET sdl2)
set (SDL2_DEFINITIONS ${PC_SDL2_CFLAGS_OTHER})

find_path (
  SDL2_INCLUDE_DIR SDL2/SDL.h
  HINTS ${PC_SDL2_INCLUDEDIR} ${PC_SDL2_INCLUDE_DIRS})

find_library (SDL2_LIBRARY
  NAMES SDL2
  HINTS ${PC_SDL2_LIBDIR} ${PC_SDL_LIBRARY_DIRS})

set (SDL2_LIBRARIES ${SDL2_LIBRARY})
set (SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})

include (FindPackageHandleStandardArgs)

find_package_handle_standard_args (
  SDL2 DEFAULT_MSG
  SDL2_LIBRARY
  SDL2_INCLUDE_DIR)

mark_as_advanced (SDL2_INCLUDE_DIR SDL2_LIBRARY)
  
  