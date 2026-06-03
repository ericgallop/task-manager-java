cmake_minimum_required(VERSION 3.14)
project(TaskManager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library target with all non-main source files (shared between CLI and tests)
add_library(task_lib
    src/task.cpp
    src/task_store.cpp
    src/task_service.cpp
)
target_include_directories(task_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Main CLI executable
add_executable(task_manager src/main.cpp)
target_link_libraries(task_manager PRIVATE task_lib)

# --- Testing with Catch2 ---
find_package(Catch2 3 REQUIRED)

add_executable(task_tests
    tests/task_service_test.cpp
)
target_link_libraries(task_tests PRIVATE task_lib Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(task_tests)
