# Set  the minimum  required version  of cmake  for a  project.
cmake_minimum_required(VERSION 2.6)

# Project name
project(POLY2TET)

# C++11
SET(CMAKE_CXX_FLAGS "-std=c++0x")

#TETGEN option
option(TETLIBRARY "use tetgen as a library" ON)
if(TETLIBRARY)
  add_definitions(-DTETLIBRARY)
endif()

# locate source files
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*)

# Add an executable to the project using the specified source files.
add_executable(${PROJECT_NAME} POLY2TET.cpp ${sources})
