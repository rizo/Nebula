cmake_minimum_required (VERSION 2.8)
project (Nebula)

set (CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -pedantic -Wno-missing-braces")
set (CMAKE_CXX_FLAGS_DEBUG "-ggdb")

add_subdirectory (gtest-1.6.0)
enable_testing ()

add_executable (nebula
  Processor.cpp
  Main.cpp)

add_executable (runTests
  tests/TestProcessor.cpp
  Processor.cpp)

target_link_libraries (runTests
  gtest
  gtest_main)
  