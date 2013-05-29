cmake_minimum_required (VERSION 2.8)
project (Nebula)

set (CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -pedantic -Wno-missing-braces")

add_subdirectory (gtest-1.6.0)
enable_testing ()

find_package (Threads REQUIRED)

include_directories (${gtest_SOURCE_DIR}/include)

add_executable (nebula
  Memory.cpp
  ProcessorState.cpp
  Computer.cpp
  Simulation/Clock.cpp
  Simulation/Processor.cpp
  Main.cpp)

target_link_libraries (nebula
  glog
  ${CMAKE_THREAD_LIBS_INIT})

add_executable (runTests
  tests/TestMemory.cpp
  tests/TestProcessorState.cpp
  Memory.cpp
  ProcessorState.cpp)

target_link_libraries (runTests
  ${CMAKE_THREAD_LIBS_INIT}
  gtest
  gtest_main)
  