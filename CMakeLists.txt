cmake_minimum_required (VERSION 2.8)
project (Nebula)

set (CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -pedantic -Wno-missing-braces")

add_subdirectory (gtest-1.6.0)
enable_testing ()

add_executable (nebula
  Memory.cpp
  Processor.cpp
  Main.cpp)

add_executable (runTests
  tests/TestMemory.cpp
  tests/TestProcessor.cpp
  Memory.cpp
  Processor.cpp)

target_link_libraries (runTests
  gtest
  gtest_main)
  