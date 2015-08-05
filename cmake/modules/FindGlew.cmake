# Try to find GLEW, optionally using pkg-config.

find_package (PkgConfig)
pkg_check_modules (PC_GLEW QUIET glew)
set (GLEW_DEFINITIONS ${PC_GLEW_CFLAGS_OTHER})

find_path (
  GLEW_INCLUDE_DIR GL/glew.h
  HINTS ${PC_GLEW_INCLUDEDIR} ${PC_GLEW_INCLUDE_DIRS})

find_library (GLEW_LIBRARY
  NAMES GLEW
  HINTS ${PC_GLEW_LIBDIR} ${PC_GLEW_LIBRARY_DIRS})

set (GLEW_LIBRARIES ${GLEW_LIBRARY})
set (GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})

include (FindPackageHandleStandardArgs)

find_package_handle_standard_args (
  Glew DEFAULT_MSG
  GLEW_LIBRARY
  GLEW_INCLUDE_DIR)

mark_as_advanced (GLEW_INCLUDE_DIR GLEW_LIBRARY)