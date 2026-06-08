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

add_library(task_manager_lib STATIC ${LIB_SOURCES})
target_include_directories(task_manager_lib PUBLIC "${CMAKE_SOURCE_DIR}/src")

# ---------------------------------------------------------------------------
# Test executable — all tests/*.cpp files, linked against doctest + lib
# ---------------------------------------------------------------------------
file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

add_executable(task_manager_tests ${TEST_SOURCES})
target_include_directories(task_manager_tests PRIVATE "${CMAKE_SOURCE_DIR}/third_party")
target_link_libraries(task_manager_tests PRIVATE task_manager_lib)

# Register with CTest so `ctest` discovers the test binary
enable_testing()
add_test(NAME unit_tests COMMAND task_manager_tests)
