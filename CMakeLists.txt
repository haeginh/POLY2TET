# Set  the minimum  required version  of cmake  for a  project.
cmake_minimum_required(VERSION 2.6)

#C++11
SET(CMAKE_CXX_FLAGS "-std=c++0x")

# Add an executable to the project using the specified source files.
add_executable(POLY2TET POLY2TET.cpp tetgen.cxx predicates.cxx ThreeVector.cpp)
