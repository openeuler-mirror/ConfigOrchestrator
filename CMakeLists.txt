cmake_minimum_required(VERSION 3.17)
project(controlpanel)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# assert usage of libfmt
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.0)
        message(FATAL_ERROR "GCC version must be at least 13.0")
    endif()
    message(STATUS "Using GCC version ${CMAKE_CXX_COMPILER_VERSION}")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.0)
        message(FATAL_ERROR "Clang version must be at least 12.0")
    endif()
    message(STATUS "Using Clang version ${CMAKE_CXX_COMPILER_VERSION}")
else()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

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

set(MAIN_SOURCES
    ${CMAKE_SOURCE_DIR}/src/controlpanel.cc
)

set(FRONT_SOURCES
    ${CMAKE_SOURCE_DIR}/src/frontend/ui_base.cc
    ${CMAKE_SOURCE_DIR}/src/frontend/main_menu.cc

    ${CMAKE_SOURCE_DIR}/src/frontend/firewall/firewall_ui.cc

    ${CMAKE_SOURCE_DIR}/src/frontend/package_manager/package_ui.cc
)

set(BACK_SOURCES
    ${CMAKE_SOURCE_DIR}/src/backend/config_backend_base.cc
    ${CMAKE_SOURCE_DIR}/src/backend/config_manager.cc

    ${CMAKE_SOURCE_DIR}/src/backend/firewall/firewall_backend.cc

    ${CMAKE_SOURCE_DIR}/src/backend/package_manager/package_manager_backend.cc
)

set(TOOL_SOURCES
    ${CMAKE_SOURCE_DIR}/src/tools/sys.cc
    ${CMAKE_SOURCE_DIR}/src/tools/iptools.cc
    ${CMAKE_SOURCE_DIR}/src/tools/uitools.cc
)

set(NON_MAIN_SOURCES
    ${TOOL_SOURCES}
    ${FRONT_SOURCES}
    ${BACK_SOURCES}
)

set(SOURCES
    ${MAIN_SOURCES}
    ${NON_MAIN_SOURCES}
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

#libyui
find_library(YUI_LIB yui REQUIRED)
find_path(YUI_INCLUDE_DIR yui REQUIRED)

# libiptc
find_library(IPTC_LIB ip4tc ip6tc REQUIRED)
find_path(IPTC_INCLUDE_DIR libiptc/libiptc.h REQUIRED)
include_directories(${IPTC_INCLUDE_DIR})

# if new lib to be linked, add it to the ALL_LIBS variable
set(ALL_LIBS ${YUI_LIB} ${IPTC_LIB})

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${ALL_LIBS})

# Testing
option(BUILD_TESTS "Build tests" ON)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
