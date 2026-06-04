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
// E2E: Start Task — happy path (TODO → IN_PROGRESS)
// =============================================================================

TEST_CASE("E2E: CLI Start Task shows IN_PROGRESS message", "[e2e][lifecycle]") {
    // 1. Add a task, 2. Start it, 3. Exit
    std::string input =
        "1\n"                     // Add task
        "Start me\n"             // Title
        "\n"                      // Default priority
        "\n"                      // No description
        "\n"                      // No due date
        "\n"                      // No assignee
        "3\n"                     // Start task
        "1\n"                     // Task ID
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Task 1 is now IN_PROGRESS.") != std::string::npos);
}

// =============================================================================
// E2E: Complete Task — happy path (TODO → DONE)
// =============================================================================

TEST_CASE("E2E: CLI Complete Task shows completed message", "[e2e][lifecycle]") {
    // 1. Add a task, 2. Complete it directly, 3. Exit
    std::string input =
        "1\n"                     // Add task
        "Complete me\n"          // Title
        "\n"                      // Default priority
        "\n"                      // No description
        "\n"                      // No due date
        "\n"                      // No assignee
        "4\n"                     // Complete task
        "1\n"                     // Task ID
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Task 1 completed.") != std::string::npos);
}

// =============================================================================
// E2E: Cancel Task — happy path (TODO → CANCELLED)
// =============================================================================

TEST_CASE("E2E: CLI Cancel Task shows cancelled message", "[e2e][lifecycle]") {
    // 1. Add a task, 2. Cancel it, 3. Exit
    std::string input =
        "1\n"                     // Add task
        "Cancel me\n"            // Title
        "\n"                      // Default priority
        "\n"                      // No description
        "\n"                      // No due date
        "\n"                      // No assignee
        "5\n"                     // Cancel task
        "1\n"                     // Task ID
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Task 1 cancelled.") != std::string::npos);
}

// =============================================================================
// E2E: Start Task on non-existent ID — not found error
// =============================================================================

TEST_CASE("E2E: CLI Start Task with missing ID shows not found", "[e2e][lifecycle]") {
    std::string input =
        "3\n"                     // Start task (no tasks exist)
        "999\n"                   // Non-existent ID
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Task not found.") != std::string::npos);
}

// =============================================================================
// E2E: Start Task on already started task — invalid transition
// =============================================================================

TEST_CASE("E2E: CLI Start Task on IN_PROGRESS task shows error", "[e2e][lifecycle]") {
    // 1. Add a task, 2. Start it, 3. Try starting again, 4. Exit
    std::string input =
        "1\n"                     // Add task
        "Double start\n"         // Title
        "\n\n\n\n"               // Default optional fields
        "3\n"                     // Start task
        "1\n"                     // Task ID
        "3\n"                     // Start task again
        "1\n"                     // Same task ID
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Task 1 is now IN_PROGRESS.") != std::string::npos);
    CHECK(output.find("Cannot start task") != std::string::npos);
    CHECK(output.find("task must be in TODO status") != std::string::npos);
}

// =============================================================================
// E2E: Complete Task on DONE task — invalid transition
// =============================================================================

TEST_CASE("E2E: CLI Complete Task on DONE task shows error", "[e2e][lifecycle]") {
    // 1. Add a task, 2. Complete it, 3. Try completing again, 4. Exit
    std::string input =
        "1\n"                     // Add task
        "Double complete\n"      // Title
        "\n\n\n\n"               // Default optional fields
        "4\n"                     // Complete task
        "1\n"                     // Task ID
        "4\n"                     // Complete task again
        "1\n"                     // Same task ID
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Task 1 completed.") != std::string::npos);
    CHECK(output.find("Cannot complete task") != std::string::npos);
    CHECK(output.find("already completed or cancelled") != std::string::npos);
}

// =============================================================================
// E2E: Cancel Task on CANCELLED task — invalid transition
// =============================================================================

TEST_CASE("E2E: CLI Cancel Task on CANCELLED task shows error", "[e2e][lifecycle]") {
    // 1. Add a task, 2. Cancel it, 3. Try cancelling again, 4. Exit
    std::string input =
        "1\n"                     // Add task
        "Double cancel\n"        // Title
        "\n\n\n\n"               // Default optional fields
        "5\n"                     // Cancel task
        "1\n"                     // Task ID
        "5\n"                     // Cancel task again
        "1\n"                     // Same task ID
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Task 1 cancelled.") != std::string::npos);
    CHECK(output.find("Cannot cancel task") != std::string::npos);
    CHECK(output.find("already completed or cancelled") != std::string::npos);
}

// =============================================================================
// E2E: Full lifecycle — TODO → IN_PROGRESS → DONE
// =============================================================================

TEST_CASE("E2E: CLI Full lifecycle TODO to IN_PROGRESS to DONE", "[e2e][lifecycle]") {
    std::string input =
        "1\n"                     // Add task
        "Lifecycle task\n"       // Title
        "\n\n\n\n"               // Default optional fields
        "3\n"                     // Start task
        "1\n"                     // Task ID
        "4\n"                     // Complete task
        "1\n"                     // Task ID
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Task 1 is now IN_PROGRESS.") != std::string::npos);
    CHECK(output.find("Task 1 completed.") != std::string::npos);
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

// =============================================================================
// E2E: Assign Task — happy path (AC1)
// =============================================================================

TEST_CASE("E2E: CLI Assign Task shows assigned message", "[e2e][update][assign]") {
    // 1. Add a task, 2. Assign it, 3. Exit
    std::string input =
        "1\n"                     // Add task
        "Assign me\n"            // Title
        "\n\n\n\n"               // Default optional fields
        "6\n"                     // Assign task
        "1\n"                     // Task ID
        "alice\n"                 // Assignee name
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Assigned task 1 to alice") != std::string::npos);
}

// =============================================================================
// E2E: Clear Assignee — empty input (AC2)
// =============================================================================

TEST_CASE("E2E: CLI Clear Assignee shows cleared message", "[e2e][update][assign]") {
    // 1. Add a task with assignee, 2. Clear the assignee, 3. Exit
    std::string input =
        "1\n"                     // Add task
        "Clear assignee\n"       // Title
        "\n"                      // Default priority
        "\n"                      // No description
        "\n"                      // No due date
        "bob\n"                   // Assignee = bob
        "6\n"                     // Assign task
        "1\n"                     // Task ID
        "\n"                      // Empty assignee → clear
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Cleared assignee from task 1") != std::string::npos);
}

// =============================================================================
// E2E: Set Priority — happy path (AC3)
// =============================================================================

TEST_CASE("E2E: CLI Set Priority shows updated message", "[e2e][update][priority]") {
    // 1. Add a task, 2. Change priority, 3. Exit
    std::string input =
        "1\n"                     // Add task
        "Priority task\n"        // Title
        "\n\n\n\n"               // Default optional fields
        "7\n"                     // Set priority
        "1\n"                     // Task ID
        "CRITICAL\n"             // New priority
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Priority updated.") != std::string::npos);
}

// =============================================================================
// E2E: Assign Task — not found (AC4)
// =============================================================================

TEST_CASE("E2E: CLI Assign Task with missing ID shows not found", "[e2e][update][assign]") {
    std::string input =
        "6\n"                     // Assign task (no tasks exist)
        "999\n"                   // Non-existent ID
        "alice\n"                 // Assignee name
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Task not found.") != std::string::npos);
}

// =============================================================================
// E2E: Set Priority — not found (AC4)
// =============================================================================

TEST_CASE("E2E: CLI Set Priority with missing ID shows not found", "[e2e][update][priority]") {
    std::string input =
        "7\n"                     // Set priority (no tasks exist)
        "999\n"                   // Non-existent ID
        "HIGH\n"                  // Priority
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Task not found.") != std::string::npos);
}

// =============================================================================
// E2E: Delete Task — confirm with lowercase 'y' (happy path)
// =============================================================================

TEST_CASE("E2E: CLI Delete Task confirmed with y removes task", "[e2e][delete]") {
    // 1. Add a task, 2. Delete it with confirmation, 3. List to verify gone, 4. Exit
    std::string input =
        "1\n"                     // Add task
        "Delete me\n"            // Title
        "\n\n\n\n"               // Default optional fields
        "8\n"                     // Remove task
        "1\n"                     // Task ID
        "y\n"                     // Confirm deletion
        "2\n"                     // List all tasks
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Are you sure you want to delete task #1? (y/n):") != std::string::npos);
    CHECK(output.find("Task 1 removed.") != std::string::npos);
    // After deletion, listing should show no tasks
    CHECK(output.find("No tasks.") != std::string::npos);
}

// =============================================================================
// E2E: Delete Task — confirm with uppercase 'Y'
// =============================================================================

TEST_CASE("E2E: CLI Delete Task confirmed with Y removes task", "[e2e][delete]") {
    // 1. Add a task, 2. Delete it with uppercase Y, 3. Exit
    std::string input =
        "1\n"                     // Add task
        "Delete uppercase\n"     // Title
        "\n\n\n\n"               // Default optional fields
        "8\n"                     // Remove task
        "1\n"                     // Task ID
        "Y\n"                     // Confirm deletion (uppercase)
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Are you sure you want to delete task #1? (y/n):") != std::string::npos);
    CHECK(output.find("Task 1 removed.") != std::string::npos);
}

// =============================================================================
// E2E: Delete Task — cancel with 'n' preserves task
// =============================================================================

TEST_CASE("E2E: CLI Delete Task cancelled with n preserves task", "[e2e][delete]") {
    // 1. Add a task, 2. Attempt delete but cancel, 3. List to verify still there, 4. Exit
    std::string input =
        "1\n"                     // Add task
        "Keep me\n"              // Title
        "\n\n\n\n"               // Default optional fields
        "8\n"                     // Remove task
        "1\n"                     // Task ID
        "n\n"                     // Reject deletion
        "2\n"                     // List all tasks
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Are you sure you want to delete task #1? (y/n):") != std::string::npos);
    CHECK(output.find("Delete cancelled.") != std::string::npos);
    // Task should still appear in list
    CHECK(output.find("Keep me") != std::string::npos);
    // "No tasks." should NOT appear
    CHECK(output.find("No tasks.") == std::string::npos);
}

// =============================================================================
// E2E: Delete Task — cancel with random input
// =============================================================================

TEST_CASE("E2E: CLI Delete Task cancelled with random input preserves task", "[e2e][delete]") {
    // 1. Add a task, 2. Attempt delete with random input, 3. Exit
    std::string input =
        "1\n"                     // Add task
        "Random cancel\n"        // Title
        "\n\n\n\n"               // Default optional fields
        "8\n"                     // Remove task
        "1\n"                     // Task ID
        "maybe\n"                // Random input → rejection
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Are you sure you want to delete task #1? (y/n):") != std::string::npos);
    CHECK(output.find("Delete cancelled.") != std::string::npos);
    // Task should NOT have been removed
    CHECK(output.find("Task 1 removed.") == std::string::npos);
}

// =============================================================================
// E2E: Delete Task — non-existent ID shows not found, no confirmation prompt
// =============================================================================

TEST_CASE("E2E: CLI Delete Task with non-existent ID shows not found", "[e2e][delete]") {
    // No tasks created — attempt to delete ID 999
    std::string input =
        "8\n"                     // Remove task (no tasks exist)
        "999\n"                   // Non-existent ID
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Task not found.") != std::string::npos);
    // Confirmation prompt should NOT appear for non-existent task
    CHECK(output.find("Are you sure") == std::string::npos);
}

// =============================================================================
// E2E: List Tasks — empty state (AC #2)
// =============================================================================

TEST_CASE("E2E: CLI List Tasks with no tasks shows empty message", "[e2e][list]") {
    std::string input =
        "2\n"                     // List all tasks (none exist)
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("No tasks.") != std::string::npos);
}

// =============================================================================
// E2E: List Tasks — sorted by priority (AC #1)
// =============================================================================

TEST_CASE("E2E: CLI List Tasks shows tasks sorted by priority CRITICAL first", "[e2e][list]") {
    // Create tasks with different priorities in non-sorted order, then list
    std::string input =
        "1\n"                     // Add task 1 (LOW)
        "Low task\n"
        "LOW\n"
        "\n\n\n"                  // No desc, due date, assignee
        "1\n"                     // Add task 2 (CRITICAL)
        "Critical task\n"
        "CRITICAL\n"
        "\n\n\n"
        "1\n"                     // Add task 3 (MEDIUM)
        "Medium task\n"
        "MEDIUM\n"
        "\n\n\n"
        "1\n"                     // Add task 4 (HIGH)
        "High task\n"
        "HIGH\n"
        "\n\n\n"
        "2\n"                     // List all tasks
        "0\n";                    // Exit

    std::string output = run_cli(input);

    // Find the positions of each task in the output after the last "Choose:" prompt
    // The list should be sorted: CRITICAL, HIGH, MEDIUM, LOW
    auto pos_critical = output.rfind("[CRITICAL]");
    auto pos_high     = output.rfind("[HIGH]");
    auto pos_medium   = output.rfind("[MEDIUM]");
    auto pos_low      = output.rfind("[LOW]");

    REQUIRE(pos_critical != std::string::npos);
    REQUIRE(pos_high != std::string::npos);
    REQUIRE(pos_medium != std::string::npos);
    REQUIRE(pos_low != std::string::npos);

    // CRITICAL should appear before HIGH, which appears before MEDIUM, etc.
    CHECK(pos_critical < pos_high);
    CHECK(pos_high < pos_medium);
    CHECK(pos_medium < pos_low);
}

// =============================================================================
// E2E: Summary — option 9 (AC #1 summary counts)
// =============================================================================

TEST_CASE("E2E: CLI Summary shows correct counts", "[e2e][list][summary]") {
    // Create tasks in various states: 1 TODO, 1 IN_PROGRESS, 1 DONE, 1 CANCELLED
    std::string input =
        "1\n"                     // Add task 1 (stays TODO)
        "TODO task\n"
        "\n\n\n\n"
        "1\n"                     // Add task 2 (will be started)
        "IP task\n"
        "\n\n\n\n"
        "3\n"                     // Start task 2
        "2\n"
        "1\n"                     // Add task 3 (will be completed)
        "Done task\n"
        "\n\n\n\n"
        "4\n"                     // Complete task 3
        "3\n"
        "1\n"                     // Add task 4 (will be cancelled)
        "Cancelled task\n"
        "\n\n\n\n"
        "5\n"                     // Cancel task 4
        "4\n"
        "9\n"                     // Show summary
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("Total: 4") != std::string::npos);
    CHECK(output.find("TODO: 1") != std::string::npos);
    CHECK(output.find("In Progress: 1") != std::string::npos);
    CHECK(output.find("Done: 1") != std::string::npos);
    CHECK(output.find("Cancelled: 1") != std::string::npos);
    CHECK(output.find("Overdue: 0") != std::string::npos);
}

// =============================================================================
// E2E: Show Overdue — option 10 with overdue task
// =============================================================================

TEST_CASE("E2E: CLI Show Overdue displays overdue tasks", "[e2e][list][overdue]") {
    // Create a task with a past due date (2020-01-01) — will be overdue
    std::string input =
        "1\n"                     // Add task
        "Overdue task\n"
        "HIGH\n"
        "\n"                      // No description
        "2020-01-01\n"           // Past due date
        "\n"                      // No assignee
        "10\n"                    // Show overdue
        "0\n";                    // Exit

    std::string output = run_cli(input);

    // The overdue task should appear in the output of option 10
    CHECK(output.find("Overdue task") != std::string::npos);
    CHECK(output.find("(due: 2020-01-01)") != std::string::npos);
    // "No overdue tasks." should NOT appear
    CHECK(output.find("No overdue tasks.") == std::string::npos);
}

// =============================================================================
// E2E: Show Overdue — option 10 with no overdue tasks
// =============================================================================

TEST_CASE("E2E: CLI Show Overdue with no overdue tasks shows empty message", "[e2e][list][overdue]") {
    // Create a task with a future due date — not overdue
    std::string input =
        "1\n"                     // Add task
        "Future task\n"
        "\n"                      // Default priority
        "\n"                      // No description
        "2099-12-31\n"           // Future due date
        "\n"                      // No assignee
        "10\n"                    // Show overdue
        "0\n";                    // Exit

    std::string output = run_cli(input);

    CHECK(output.find("No overdue tasks.") != std::string::npos);
}
