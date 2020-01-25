cmake_minimum_required(VERSION 3.10)
project(caboose LANGUAGES C VERSION 2.0.0)

set(CMAKE_C_STANDARD 11)

file(GLOB lib_source "src/*.c")
file(GLOB lib_header "src/*.h")
#file(GLOB linenoise_source_header "src/app/linenoise/*.c" "src/app/linenoise/*.h")

add_library(caboose SHARED ${lib_source})
add_executable(cb src/app/main.c ${lib_source} ${lib_header})
target_link_libraries(cb caboose)

file(GLOB lib_source)

set_target_properties(caboose PROPERTIES PUBLIC_HEADER "chunk.h;debug.h;value.h;memory.h")
INSTALL(TARGETS caboose LIBRARY DESTINATION lib/caboose PUBLIC_HEADER DESTINATION include/caboose)

enable_testing()

add_test(fun ./cb test_fun.cb)
