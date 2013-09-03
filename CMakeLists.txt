cmake_minimum_required (VERSION 2.8)
project (Nebula)

set (CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -pedantic -Wno-missing-braces -march=native -mtune=native")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 ${CMAKE_CXX_FLAGS_RELEASE}")

add_subdirectory (gtest-1.6.0)
enable_testing ()

find_package (Boost REQUIRED COMPONENTS log log_setup thread system program_options)
find_package (Threads REQUIRED)
find_package (SDL REQUIRED)

add_definitions (-DBOOST_ALL_DYN_LINK)

include_directories (${gtest_SOURCE_DIR}/include)

add_executable (nebula
  Fundamental.cpp
  Memory.cpp
  ProcessorState.cpp
  Computer.cpp
  Sdl.cpp
  Simulation/Clock.cpp
  Simulation/FloppyDrive.cpp
  Simulation/Processor.cpp
  Simulation/Monitor.cpp
  Simulation/Keyboard.cpp
  Main.cpp)

target_link_libraries (nebula
  ${SDL_LIBRARY}
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT})

add_executable (runTests
  Tests/TestMemory.cpp
  Tests/TestProcessorState.cpp
  Memory.cpp
  ProcessorState.cpp)

target_link_libraries (runTests
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  gtest
  gtest_main)
  
install (TARGETS nebula
  RUNTIME DESTINATION bin)