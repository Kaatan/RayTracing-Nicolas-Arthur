cmake_minimum_required (VERSION 3.0)

set(CMAKE_C_COMPILER "C:/MinGW/bin/gcc")
set(CMAKE_CXX_COMPILER "C:/MinGW/bin/g++")

project (ProjectTest)


##############
## EXTERNAL ##
##############
add_subdirectory(External/glfw)


##############
## PROJECT ##
##############

file(GLOB_RECURSE WHATEVER_FILES
  src/*
  )

include_directories(include)

add_executable(whatever ${WHATEVER_FILES})

target_link_libraries(whatever glfw)

target_include_directories(whatever
  SYSTEM PUBLIC
  External/glfw/include/
)

target_include_directories(whatever
  PUBLIC
  include/
)