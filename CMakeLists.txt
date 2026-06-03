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

add_executable(cli_e2e_tests
    tests/cli_e2e_test.cpp
)
target_link_libraries(cli_e2e_tests PRIVATE task_lib Catch2::Catch2WithMain)

# E2E tests need the CLI binary to be built first
add_dependencies(cli_e2e_tests task_manager)

include(CTest)
include(Catch)
catch_discover_tests(task_tests)
catch_discover_tests(cli_e2e_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
