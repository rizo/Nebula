# Try to find GlFW, optionally using pkg-config.

find_package (PkgConfig)
pkg_check_modules(PC_GLFW QUIET glfw-3.0)
set (GLFW_DEFINITIONS ${PC_FLGW_CFLAGS_OTHER})

find_path (
  GLFW_INCLUDE_DIR GLFW/glfw3.h
  HINTS ${PC_GLFW_INCLUDEDIR} ${PC_GLFW_INCLUDE_DIRS})

find_library (
  GLFW_LIBRARY NAMES glfw
  HINTS ${PC_GLFW_LIBDIR} ${PC_GLFW_LIBRARY_DIRS})

set (GLFW_LIBRARIES ${GLFW_LIBRARY})
set (GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})

include (FindPackageHandleStandardArgs)

find_package_handle_standard_args (
  Glfw DEFAULT_MSG
  GLFW_LIBRARY
  GLFW_INCLUDE_DIR)

mark_as_advanced (GLFW_INCLUDE_DIR GLFW_LIBRARY)

  
  