find_library(GTEST_LIB gtest)
find_path(GTEST_INCLUDE_DIR gtest/gtest.h)

if (GTEST_LIB AND GTEST_INCLUDE_DIR)
    message(STATUS "found gtest libraries and include directory")
else()
    message(FATAL_ERROR "gtest libraries or include directory not found")
endif()

include_directories(${GTEST_INCLUDE_DIR})

function(add_gtest test_name test_source)
    add_executable(${test_name} ${test_source} ${NON_MAIN_SOURCES})
    target_link_libraries(${test_name} ${GTEST_LIB} ${ALL_LIBS} pthread)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_subdirectory(backend)
