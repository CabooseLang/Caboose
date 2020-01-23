cmake_minimum_required(VERSION 3.15)
project(caboose LANGUAGES C VERSION 2.0.0)

set(CMAKE_C_STANDARD 11)

file(GLOB lib_source "src/*.c")
file(GLOB lib_header "src/*.h")

add_library(caboose SHARED ${lib_source})

install(TARGETS caboose DESTINATION lib)
install(FILES ${lib_header} DESTINATION include/caboose)
