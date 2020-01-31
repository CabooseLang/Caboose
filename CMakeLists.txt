cmake_minimum_required(VERSION 3.10)

include(cmake/VersionFromGit.cmake)
include(cmake/ClangFormat.cmake)

version_from_git()
project(Caboose LANGUAGES C VERSION ${git_resolved_version})
set(CMAKE_C_STANDARD 11)

file(GLOB lib_source "src/*.c")
file(GLOB lib_header "src/*.h")

add_library(caboose STATIC ${lib_source})
add_executable(cb src/app/main.c ${lib_source} ${lib_header})
target_link_libraries(cb caboose)

set_target_properties(caboose PROPERTIES PUBLIC_HEADER "src/chunk.h;src/debug.h;src/value.h;src/memory.h")
install(TARGETS caboose LIBRARY DESTINATION lib/caboose PUBLIC_HEADER DESTINATION include/caboose ARCHIVE DESTINATION lib/caboose)
install(TARGETS cb DESTINATION bin)

enable_testing()

add_test(fun ./cb test_fun.cb)

# Packaging
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${caboose_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${caboose_VERSION_MINOR}")
set(CPACK_PACKAGE_CONTACT "Caboose Maintainers")
set(CPACK_PACKAGE_FILE_NAME "Caboose-${CMAKE_PROJECT_VERSION}")
include(CPack)

