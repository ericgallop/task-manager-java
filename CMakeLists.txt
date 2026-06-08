cmake_minimum_required(VERSION 3.16)
project(task_manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# ---------------------------------------------------------------------------
# Production library — all src/*.cpp files
# ---------------------------------------------------------------------------
file(GLOB LIB_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

if(LIB_SOURCES)
    add_library(task_manager_lib ${LIB_SOURCES})
    target_include_directories(task_manager_lib PUBLIC "${CMAKE_SOURCE_DIR}/src")
else()
    # Header-only placeholder so tests can still reference include dirs
    add_library(task_manager_lib INTERFACE)
    target_include_directories(task_manager_lib INTERFACE "${CMAKE_SOURCE_DIR}/src")
endif()

# ---------------------------------------------------------------------------
# Test executable — all tests/*.cpp files, linked against doctest + lib
# ---------------------------------------------------------------------------
enable_testing()

file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

if(TEST_SOURCES)
    add_executable(task_manager_tests ${TEST_SOURCES})
    target_link_libraries(task_manager_tests PRIVATE task_manager_lib)

    # doctest header-only framework (include third_party/ so #include "doctest/doctest.h" works)
    target_include_directories(task_manager_tests PRIVATE "${CMAKE_SOURCE_DIR}/third_party")
    target_include_directories(task_manager_tests PRIVATE "${CMAKE_SOURCE_DIR}/src")

    add_test(NAME task_manager_tests COMMAND task_manager_tests)
endif()
