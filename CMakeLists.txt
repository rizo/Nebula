## /nebula/CMakeLists.txt
## Copyright 2014 Jesse Haber-Kucharsky
##
## Released under the terms of the Apache License Version 2.0
## See /nebula/LICENSE or http://www.apache.org/licenses/

cmake_minimum_required (VERSION 2.8)
project (nebula)

set (CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

if (UNIX)  
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra -pedantic -Werror")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic -Werror")
endif (UNIX)

add_definitions (
  -DGLM_FORCE_RADIANS
  -DBOOST_ALL_DYN_LINK
  -DBOOST_SPIRIT_USE_PHOENIX_V3=1)

find_package (Boost REQUIRED
  COMPONENTS log log_setup thread system program_options)
# For consistency.
set (BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
set (BOOST_LIBRARIES ${Boost_LIBRARIES}) 

find_package (Threads REQUIRED)

find_package (Sdl2 REQUIRED)

find_package (Glew REQUIRED)

find_package (OpenGL REQUIRED)

find_package (Glm REQUIRED)

include_directories (SYSTEM
  ${BOOST_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS})

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  "${CMAKE_CURRENT_SOURCE_DIR}/src")

set (PLATFORM_SOURCE
  src/platform/address.cpp
  src/platform/audio.cpp
  src/platform/computer.cpp
  src/platform/decode.cpp
  src/platform/event.cpp
  src/platform/execution-manager.cpp
  src/platform/graphics.cpp
  src/platform/instruction.cpp
  src/platform/interactive.cpp
  src/platform/interrupt.cpp
  src/platform/memory.cpp
  src/platform/prelude.cpp
  src/platform/processor-state.cpp)

set (DEVICE_SOURCE
  src/device/clock.cpp
  src/device/keyboard.cpp
  src/device/monitor.cpp
  src/device/synthesizer.cpp
  src/device/vector-display.cpp)

set (MAIN_SOURCE
  src/main.cpp)

add_executable (nebula
  ${DEVICE_SOURCE}
  ${PLATFORM_SOURCE}
  ${MAIN_SOURCE})

target_link_libraries (nebula
  ${CMAKE_THREAD_LIBS_INIT}
  ${BOOST_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${OPENGL_LIBRARIES})

install (TARGETS nebula
  RUNTIME DESTINATION bin)

##
## Experiments
##

add_executable (exp-sound
  ${PLATFORM_SOURCE}
  src/experimental/sound.cpp)

target_link_libraries (exp-sound
  ${CMAKE_THREAD_LIBS_INIT}
  ${BOOST_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${OPENGL_LIBRARIES})