cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Caboose VERSION 1.0.0)
file(GLOB LIB_SOURCES "src/*.c")

add_executable(caboose ${LIB_SOURCES})
install(TARGETS caboose DESTINATION bin)