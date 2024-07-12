cmake_minimum_required(VERSION 3.17)
project(controlpane)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Building in Release mode")
else()
    message("Building in Debug mode")
    option()(ADDRESS_SANITIZER "Enable AddressSanitizer" ON)
    option(THREAD_SANITIZER "Enable ThreadSanitizer" OFF)

    if(THREAD_SANITIZER)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls")
        message(STATUS "ThreadSanitizer is enabled.")
    elseif(ADDRESS_SANITIZER)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined  -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls")
        message(STATUS "AddressSanitizer is enabled.")
    endif()
endif()

add_custom_target(build_libyui ALL
  COMMAND ${CMAKE_COMMAND} -E echo "Building libyui..."
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}/libyui ./build_all
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libyui
)

link_directories(${CMAKE_SOURCE_DIR}/libyui/build)
include_directories(${CMAKE_SOURCE_DIR}/libyui/include)

add_executable(controlpane main.cc)
target_link_libraries(controlpane PRIVATE yui)
add_dependencies(controlpane build_libyui)
