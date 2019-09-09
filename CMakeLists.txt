cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Caboose VERSION 1.1.0)
file(GLOB LIB_SOURCES "src/*.c")

add_executable(caboose ${LIB_SOURCES})
target_link_libraries(caboose m)
install(TARGETS caboose DESTINATION bin)

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
elseif( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "DEB;RPM")
endif()

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Caboose - A simple but powerful interpreted language written in C")
set(CPACK_PACKAGE_VENDOR "Caboose Authors")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Caboose Authors")

INCLUDE(CPack)