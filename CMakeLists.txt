cmake_minimum_required(VERSION 3.13)
project(footswitch C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)
set(SRCDIR src)

find_package(PkgConfig)
pkg_check_modules(HIDAPI REQUIRED hidapi-libusb)

include_directories(${HIDAPI_INCLUDE_DIRS} include)
link_libraries(${HIDAPI_LIBRARIES})
link_directories(src)
