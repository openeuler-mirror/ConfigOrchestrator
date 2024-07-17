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
    option(ADDRESS_SANITIZER "Enable AddressSanitizer" ON)
    option(THREAD_SANITIZER "Enable ThreadSanitizer" OFF)

    if(THREAD_SANITIZER)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls")
        message(STATUS "ThreadSanitizer is enabled.")
    elseif(ADDRESS_SANITIZER)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined  -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls")
        message(STATUS "AddressSanitizer is enabled.")
    endif()
endif()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if(CLANG_TIDY_EXE)
  message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
else()
  message(STATUS "clang-tidy not found.")
endif()

set(SOURCES src/controlpane.cc

            src/backend/config_registry.cc
            src/backend/config_item_base.cc
            src/backend/firewall_config.cc
            src/backend/package_config.cc

            src/frontend/ui_base.cc
)

find_program(CLANG_FORMAT_EXE NAMES clang-format)
if(CLANG_FORMAT_EXE)
  message(STATUS "Found clang-format: ${CLANG_FORMAT_EXE}")
  add_custom_target(
    clang_format
    COMMAND ${CLANG_FORMAT_EXE} -i ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running clang-format on source files"
  )
else()
  message(STATUS "clang-format not found.")
endif()

include_directories(include)
include_directories(/usr/include/yui)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} yui)
