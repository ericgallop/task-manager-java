#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include "date_utils.h"

// Helper: run the CLI binary with piped stdin and capture stdout.
// stderr is suppressed (redirected to /dev/null) so validation errors
// don't pollute the captured output.
static std::string run_cli(const std::string& input) {
    // Write input to a temp file
    std::string tmpfile = "/tmp/cli_e2e_input_" + std::to_string(getpid());
    {
        FILE* f = fopen(tmpfile.c_str(), "w");
        if (!f) throw std::runtime_error("Cannot create temp file");
        fwrite(input.data(), 1, input.size(), f);
        fclose(f);
    }

    std::string cmd = "./task_manager 2>/dev/null < " + tmpfile;
    std::array<char, 4096> buffer;
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed");

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);

    // Cleanup
    std::remove(tmpfile.c_str());
    return result;
}

// Helper: run CLI and capture stderr only.
static std::string run_cli_stderr(const std::string& input) {
    std::string tmpfile = "/tmp/cli_e2e_input_" + std::to_string(getpid());
    {
        FILE* f = fopen(tmpfile.c_str(), "w");
        if (!f) throw std::runtime_error("Cannot create temp file");
        fwrite(input.data(), 1, input.size(), f);
        fclose(f);
    }

    std::string cmd = "./task_manager 1>/dev/null 2>&1 < " + tmpfile;

    // Actually we want stderr only:
    cmd = "./task_manager 2>&1 1>/dev/null < " + tmpfile;

    std::array<char, 4096> buffer;
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed");

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);

    std::remove(tmpfile.c_str());
    return result;
}

// =============================================================================
// E2E: Add Task via CLI — happy path with all fields
// =============================================================================

TEST_CASE("E2E: CLI Add Task with all fields shows Created output", "[e2e][create]") {
    std::string input =
        "1\n"                     // Choose "Add task"
        "Buy groceries\n"        // Title
        "HIGH\n"                  // Priority
        "Need milk and eggs\n"   // Description
        "2025-12-31\n"           // Due date
        "Alice\n"                 // Assignee
        "0\n";                    // Exit

    std::string output = run_cli(input);

    // Verify the "Created:" line appears with correct task details
    CHECK(output.find("Created:") != std::string::npos);
    CHECK(output.find("[TODO]") != std::string::npos);
    CHECK(output.find("#1") != std::string::npos);
    CHECK(output.find("[HIGH]") != std::string::npos);
    CHECK(output.find("Buy groceries") != std::string::npos);
    CHECK(output.find("(due: 2025-12-31)") != std::string::npos);
    CHECK(output.find("[@Alice]") != std::string::npos);
    CHECK(output.find("Goodbye!") != std::string::npos);
}

// =============================================================================
// E2E: Add Task via CLI — default priority
// =============================================================================

TEST_CASE("E2E: CLI Add Task with default priority shows MEDIUM", "[e2e][create]") {
    std::string input =
        "1\n"                 // Choose "Add task"
        "Simple task\n"       // Title
        "\n"                  // Empty priority → default MEDIUM
        "\n"                  // No description
        "\n"                  // No due date
        "\n"                  // No assignee
        "0\n";                // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Created:") != std::string::npos);
    CHECK(output.find("[MEDIUM]") != std::string::npos);
    CHECK(output.find("Simple task") != std::string::npos);
    CHECK(output.find("#1") != std::string::npos);
}

// =============================================================================
// E2E: Add Task with blank title — error, no "Created:" output
// =============================================================================

TEST_CASE("E2E: CLI Add Task with blank title does not create task", "[e2e][create][validation]") {
    std::string input =
        "1\n"      // Choose "Add task"
        "\n"       // Empty title
        "HIGH\n"   // Priority (will be read but task should fail)
        "\n"       // Description
        "\n"       // Due date
        "\n"       // Assignee
        "0\n";     // Exit

    std::string output = run_cli(input);

    // "Created:" should NOT appear in stdout
    CHECK(output.find("Created:") == std::string::npos);

    // The error message should appear on stderr
    std::string err = run_cli_stderr(input);
    CHECK(err.find("Title cannot be empty.") != std::string::npos);
}
