#include <catch2/catch_test_macros.hpp>
#include "task_service.h"
#include "task_store.h"
#include "priority.h"
#include "date_utils.h"

// =============================================================================
// Task Creation — Default values (AC #1)
// Ported from: TaskManagerTest.addTask_createsTaskWithTitle
//              TaskServiceTest.createTask_defaultsMediumPriority
// =============================================================================

TEST_CASE("createTask with just title sets correct defaults", "[create]") {
    TaskStore store;
    TaskService service(store);

    auto result = service.createTask("Write tests");

    REQUIRE(result.has_value());
    auto task = result.value();

    CHECK(task.get_id() == 1);
    CHECK(task.get_title() == "Write tests");
    CHECK(task.get_status() == TaskStatus::TODO);
    CHECK(task.get_priority() == Priority::MEDIUM);
    CHECK(task.get_created_date() == today_as_string());
    CHECK(task.get_description() == "");
    CHECK(task.get_due_date() == "");
    CHECK(task.get_assignee() == "");
    CHECK_FALSE(task.is_completed());
}

// =============================================================================
// Task Creation — All fields (AC #2, #3, #4, #5)
// Ported from: TaskServiceTest.createTask_withAllFields
// =============================================================================

TEST_CASE("createTask with all fields stores them correctly", "[create]") {
    TaskStore store;
    TaskService service(store);

    auto result = service.createTask(
        "Full task", Priority::HIGH, "A description", "2025-12-31", "Alice");

    REQUIRE(result.has_value());
    auto task = result.value();

    CHECK(task.get_title() == "Full task");
    CHECK(task.get_priority() == Priority::HIGH);
    CHECK(task.get_description() == "A description");
    CHECK(task.get_due_date() == "2025-12-31");
    CHECK(task.get_assignee() == "Alice");
    CHECK(task.get_status() == TaskStatus::TODO);
    CHECK(task.get_created_date() == today_as_string());
}

// =============================================================================
// Auto-incrementing IDs (AC #1)
// Ported from: TaskManagerTest.addTask_assignsIncrementingIds
// =============================================================================

TEST_CASE("createTask assigns auto-incrementing IDs", "[create]") {
    TaskStore store;
    TaskService service(store);

    auto first = service.createTask("First");
    auto second = service.createTask("Second");

    REQUIRE(first.has_value());
    REQUIRE(second.has_value());
    CHECK(first->get_id() == 1);
    CHECK(second->get_id() == 2);
    CHECK(second->get_id() == first->get_id() + 1);
}

// =============================================================================
// Blank title rejected (AC #6)
// =============================================================================

TEST_CASE("createTask rejects empty title", "[create][validation]") {
    TaskStore store;
    TaskService service(store);

    auto result = service.createTask("");

    CHECK_FALSE(result.has_value());
    // Should not have saved anything
    CHECK(service.getAllTasks().empty());
}

TEST_CASE("createTask rejects whitespace-only title", "[create][validation]") {
    TaskStore store;
    TaskService service(store);

    auto result1 = service.createTask("   ");
    CHECK_FALSE(result1.has_value());

    auto result2 = service.createTask(" \t\n\r ");
    CHECK_FALSE(result2.has_value());

    // No tasks should have been saved
    CHECK(service.getAllTasks().empty());
}

// =============================================================================
// Priority parsing (AC #2)
// =============================================================================

TEST_CASE("priority_from_string parses valid values case-insensitively", "[priority]") {
    CHECK(priority_from_string("LOW") == Priority::LOW);
    CHECK(priority_from_string("low") == Priority::LOW);
    CHECK(priority_from_string("Low") == Priority::LOW);

    CHECK(priority_from_string("MEDIUM") == Priority::MEDIUM);
    CHECK(priority_from_string("medium") == Priority::MEDIUM);
    CHECK(priority_from_string("Medium") == Priority::MEDIUM);

    CHECK(priority_from_string("HIGH") == Priority::HIGH);
    CHECK(priority_from_string("high") == Priority::HIGH);
    CHECK(priority_from_string("High") == Priority::HIGH);

    CHECK(priority_from_string("CRITICAL") == Priority::CRITICAL);
    CHECK(priority_from_string("critical") == Priority::CRITICAL);
    CHECK(priority_from_string("CrItIcAl") == Priority::CRITICAL);
}

TEST_CASE("priority_from_string defaults to MEDIUM for unrecognized input", "[priority]") {
    CHECK(priority_from_string("") == Priority::MEDIUM);
    CHECK(priority_from_string("urgent") == Priority::MEDIUM);
    CHECK(priority_from_string("MED") == Priority::MEDIUM);
    CHECK(priority_from_string("   ") == Priority::MEDIUM);
    CHECK(priority_from_string("123") == Priority::MEDIUM);
}

TEST_CASE("priority_to_string round-trips correctly", "[priority]") {
    CHECK(priority_to_string(Priority::LOW) == "LOW");
    CHECK(priority_to_string(Priority::MEDIUM) == "MEDIUM");
    CHECK(priority_to_string(Priority::HIGH) == "HIGH");
    CHECK(priority_to_string(Priority::CRITICAL) == "CRITICAL");
}

// =============================================================================
// Date validation (AC #4)
// =============================================================================

TEST_CASE("is_valid_date accepts valid YYYY-MM-DD dates", "[date]") {
    CHECK(is_valid_date("2025-01-15"));
    CHECK(is_valid_date("2024-12-31"));
    CHECK(is_valid_date("2024-01-01"));
    CHECK(is_valid_date("2000-06-15"));
    CHECK(is_valid_date("1999-12-31"));
}

TEST_CASE("is_valid_date rejects invalid dates", "[date]") {
    // Invalid month
    CHECK_FALSE(is_valid_date("2024-13-01"));
    CHECK_FALSE(is_valid_date("2024-00-15"));

    // Invalid day
    CHECK_FALSE(is_valid_date("2024-01-00"));
    CHECK_FALSE(is_valid_date("2024-01-32"));

    // Wrong format (no zero-padding)
    CHECK_FALSE(is_valid_date("2024-1-5"));

    // Non-date strings
    CHECK_FALSE(is_valid_date("not-a-date"));
    CHECK_FALSE(is_valid_date("abcd-ef-gh"));

    // Empty string
    CHECK_FALSE(is_valid_date(""));

    // Has time component
    CHECK_FALSE(is_valid_date("2024-06-15T12:00"));

    // Too short / too long
    CHECK_FALSE(is_valid_date("2024-01"));
    CHECK_FALSE(is_valid_date("2024-01-015"));
}

TEST_CASE("createTask with valid due date stores it", "[create][date]") {
    TaskStore store;
    TaskService service(store);

    auto result = service.createTask("Dated task", Priority::MEDIUM, "", "2025-01-15", "");

    REQUIRE(result.has_value());
    CHECK(result->get_due_date() == "2025-01-15");
}

TEST_CASE("createTask with invalid due date returns nullopt", "[create][date][validation]") {
    TaskStore store;
    TaskService service(store);

    auto result1 = service.createTask("Bad date 1", Priority::MEDIUM, "", "2024-13-01", "");
    CHECK_FALSE(result1.has_value());

    auto result2 = service.createTask("Bad date 2", Priority::MEDIUM, "", "2024-1-5", "");
    CHECK_FALSE(result2.has_value());

    auto result3 = service.createTask("Bad date 3", Priority::MEDIUM, "", "not-a-date", "");
    CHECK_FALSE(result3.has_value());

    // No tasks should have been saved
    CHECK(service.getAllTasks().empty());
}

TEST_CASE("createTask with empty due date is valid (no due date)", "[create][date]") {
    TaskStore store;
    TaskService service(store);

    auto result = service.createTask("No date task", Priority::LOW, "desc", "", "Bob");

    REQUIRE(result.has_value());
    CHECK(result->get_due_date() == "");
}

// =============================================================================
// Optional fields (AC #3, #5)
// =============================================================================

TEST_CASE("createTask with empty optional fields stores empty strings", "[create]") {
    TaskStore store;
    TaskService service(store);

    auto result = service.createTask("Minimal task", Priority::MEDIUM, "", "", "");

    REQUIRE(result.has_value());
    CHECK(result->get_description() == "");
    CHECK(result->get_due_date() == "");
    CHECK(result->get_assignee() == "");
}

// =============================================================================
// Task persisted in store
// =============================================================================

TEST_CASE("createTask persists task retrievable via getTask", "[create][store]") {
    TaskStore store;
    TaskService service(store);

    auto created = service.createTask("Persistent task");
    REQUIRE(created.has_value());

    auto retrieved = service.getTask(created->get_id());
    REQUIRE(retrieved.has_value());
    CHECK(retrieved->get_id() == created->get_id());
    CHECK(retrieved->get_title() == "Persistent task");
    CHECK(retrieved->get_status() == TaskStatus::TODO);
    CHECK(retrieved->get_priority() == Priority::MEDIUM);
}

TEST_CASE("createTask tasks appear in getAllTasks", "[create][store]") {
    TaskStore store;
    TaskService service(store);

    service.createTask("Task A");
    service.createTask("Task B");
    service.createTask("Task C");

    auto all = service.getAllTasks();
    CHECK(all.size() == 3);
}

// =============================================================================
// Delete task
// Ported from: TaskManagerTest.removeTask_deletesTask
//              TaskServiceTest.deleteTask_removesTask
// =============================================================================

TEST_CASE("deleteTask removes task from store", "[delete]") {
    TaskStore store;
    TaskService service(store);

    auto task = service.createTask("Delete me");
    REQUIRE(task.has_value());

    CHECK(service.deleteTask(task->get_id()));
    CHECK(service.getAllTasks().empty());
    CHECK_FALSE(service.getTask(task->get_id()).has_value());
}

TEST_CASE("deleteTask returns false for non-existent ID", "[delete]") {
    TaskStore store;
    TaskService service(store);

    CHECK_FALSE(service.deleteTask(999));
}

// =============================================================================
// today_as_string utility
// =============================================================================

TEST_CASE("today_as_string returns valid YYYY-MM-DD format", "[date]") {
    std::string today = today_as_string();
    CHECK(today.size() == 10);
    CHECK(is_valid_date(today));
}

// =============================================================================
// Lifecycle State Transitions — Ported from Java
// Ported from: TaskServiceTest.startTask_changesStatusToInProgress
//              TaskServiceTest.completeTask_fromInProgress
//              TaskServiceTest.cancelTask_fromTodo
//              TaskServiceTest.startTask_failsForMissingId
// =============================================================================

TEST_CASE("startTask changes status to IN_PROGRESS", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());

    CHECK(service.startTask(task->get_id()) == TransitionResult::Success);

    auto updated = service.getTask(task->get_id());
    REQUIRE(updated.has_value());
    CHECK(updated->get_status() == TaskStatus::IN_PROGRESS);
}

TEST_CASE("completeTask from IN_PROGRESS changes status to DONE", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());

    REQUIRE(service.startTask(task->get_id()) == TransitionResult::Success);
    CHECK(service.completeTask(task->get_id()) == TransitionResult::Success);

    auto updated = service.getTask(task->get_id());
    REQUIRE(updated.has_value());
    CHECK(updated->get_status() == TaskStatus::DONE);
}

TEST_CASE("cancelTask from TODO changes status to CANCELLED", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());

    CHECK(service.cancelTask(task->get_id()) == TransitionResult::Success);

    auto updated = service.getTask(task->get_id());
    REQUIRE(updated.has_value());
    CHECK(updated->get_status() == TaskStatus::CANCELLED);
}

TEST_CASE("startTask returns NotFound for missing ID", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);

    CHECK(service.startTask(999) == TransitionResult::NotFound);
}

// =============================================================================
// Lifecycle State Transitions — New tests required by Acceptance Criteria
// =============================================================================

// AC #2: TODO → DONE (direct completion without starting)
TEST_CASE("completeTask from TODO changes status to DONE", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());

    CHECK(service.completeTask(task->get_id()) == TransitionResult::Success);

    auto updated = service.getTask(task->get_id());
    REQUIRE(updated.has_value());
    CHECK(updated->get_status() == TaskStatus::DONE);
}

// AC #3: IN_PROGRESS → CANCELLED
TEST_CASE("cancelTask from IN_PROGRESS changes status to CANCELLED", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());

    REQUIRE(service.startTask(task->get_id()) == TransitionResult::Success);
    CHECK(service.cancelTask(task->get_id()) == TransitionResult::Success);

    auto updated = service.getTask(task->get_id());
    REQUIRE(updated.has_value());
    CHECK(updated->get_status() == TaskStatus::CANCELLED);
}

// AC #4: DONE tasks cannot be modified
TEST_CASE("startTask on DONE task returns InvalidTransition", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());

    REQUIRE(service.completeTask(task->get_id()) == TransitionResult::Success);
    CHECK(service.startTask(task->get_id()) == TransitionResult::InvalidTransition);
}

TEST_CASE("completeTask on DONE task returns InvalidTransition", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());

    REQUIRE(service.completeTask(task->get_id()) == TransitionResult::Success);
    CHECK(service.completeTask(task->get_id()) == TransitionResult::InvalidTransition);
}

TEST_CASE("cancelTask on DONE task returns InvalidTransition", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());

    REQUIRE(service.completeTask(task->get_id()) == TransitionResult::Success);
    CHECK(service.cancelTask(task->get_id()) == TransitionResult::InvalidTransition);
}

// AC #5: CANCELLED tasks cannot be modified
TEST_CASE("startTask on CANCELLED task returns InvalidTransition", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());

    REQUIRE(service.cancelTask(task->get_id()) == TransitionResult::Success);
    CHECK(service.startTask(task->get_id()) == TransitionResult::InvalidTransition);
}

TEST_CASE("completeTask on CANCELLED task returns InvalidTransition", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());

    REQUIRE(service.cancelTask(task->get_id()) == TransitionResult::Success);
    CHECK(service.completeTask(task->get_id()) == TransitionResult::InvalidTransition);
}

TEST_CASE("cancelTask on CANCELLED task returns InvalidTransition", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());

    REQUIRE(service.cancelTask(task->get_id()) == TransitionResult::Success);
    CHECK(service.cancelTask(task->get_id()) == TransitionResult::InvalidTransition);
}

// AC #6: IN_PROGRESS task cannot be started again
TEST_CASE("startTask on IN_PROGRESS task returns InvalidTransition", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());

    REQUIRE(service.startTask(task->get_id()) == TransitionResult::Success);
    CHECK(service.startTask(task->get_id()) == TransitionResult::InvalidTransition);
}

// AC #7: NotFound for missing IDs (complete and cancel)
TEST_CASE("completeTask returns NotFound for missing ID", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);

    CHECK(service.completeTask(999) == TransitionResult::NotFound);
}

TEST_CASE("cancelTask returns NotFound for missing ID", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);

    CHECK(service.cancelTask(999) == TransitionResult::NotFound);
}

// =============================================================================
// Update Task Properties — Assign & Priority
// Ported from: TaskServiceTest.assignTask_setsAssignee
//              TaskServiceTest.updatePriority_changesPriority
// Plus new tests for clearing assignee and not-found cases
// =============================================================================

// AC1: Assign by name
TEST_CASE("assignTask sets assignee", "[update][assign]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());

    CHECK(service.assignTask(task->get_id(), "alice"));

    auto updated = service.getTask(task->get_id());
    REQUIRE(updated.has_value());
    CHECK(updated->get_assignee() == "alice");
}

// AC2: Clear assignee with empty string
TEST_CASE("assignTask with empty string clears assignee", "[update][assign]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task", Priority::MEDIUM, "", "", "bob");
    REQUIRE(task.has_value());
    REQUIRE(task->get_assignee() == "bob");

    CHECK(service.assignTask(task->get_id(), ""));

    auto updated = service.getTask(task->get_id());
    REQUIRE(updated.has_value());
    CHECK(updated->get_assignee() == "");
}

// AC3: Change priority
TEST_CASE("updatePriority changes priority", "[update][priority]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());
    REQUIRE(task->get_priority() == Priority::MEDIUM);

    CHECK(service.updatePriority(task->get_id(), Priority::CRITICAL));

    auto updated = service.getTask(task->get_id());
    REQUIRE(updated.has_value());
    CHECK(updated->get_priority() == Priority::CRITICAL);
}

// AC4: Assign returns false for non-existent ID
TEST_CASE("assignTask returns false for missing ID", "[update][assign]") {
    TaskStore store;
    TaskService service(store);

    CHECK_FALSE(service.assignTask(999, "alice"));
}

// AC4: Update priority returns false for non-existent ID
TEST_CASE("updatePriority returns false for missing ID", "[update][priority]") {
    TaskStore store;
    TaskService service(store);

    CHECK_FALSE(service.updatePriority(999, Priority::HIGH));
}

// =============================================================================
// View & List Tasks — getTasksSortedByPriority
// Ported from: TaskServiceTest.getTasksSortedByPriority
// =============================================================================

TEST_CASE("getTasksSortedByPriority returns tasks ordered CRITICAL first", "[list][sort]") {
    TaskStore store;
    TaskService service(store);

    // Create tasks with mixed priorities (deliberately not in order)
    service.createTask("Low task", Priority::LOW, "", "", "");
    service.createTask("Critical task", Priority::CRITICAL, "", "", "");
    service.createTask("Medium task", Priority::MEDIUM, "", "", "");
    service.createTask("High task", Priority::HIGH, "", "", "");

    auto sorted = service.getTasksSortedByPriority();
    REQUIRE(sorted.size() == 4);
    CHECK(sorted[0].get_priority() == Priority::CRITICAL);
    CHECK(sorted[0].get_title() == "Critical task");
    CHECK(sorted[1].get_priority() == Priority::HIGH);
    CHECK(sorted[1].get_title() == "High task");
    CHECK(sorted[2].get_priority() == Priority::MEDIUM);
    CHECK(sorted[2].get_title() == "Medium task");
    CHECK(sorted[3].get_priority() == Priority::LOW);
    CHECK(sorted[3].get_title() == "Low task");
}

TEST_CASE("getTasksSortedByPriority returns empty vector when no tasks exist", "[list][sort]") {
    TaskStore store;
    TaskService service(store);

    auto sorted = service.getTasksSortedByPriority();
    CHECK(sorted.empty());
}

// =============================================================================
// View & List Tasks — getOverdueTasks
// =============================================================================

TEST_CASE("getOverdueTasks returns only overdue tasks", "[list][overdue]") {
    TaskStore store;
    TaskService service(store);

    // Overdue TODO task (past due date, non-terminal status)
    service.createTask("Overdue TODO", Priority::MEDIUM, "", "2020-01-01", "");
    // Overdue IN_PROGRESS task
    auto ip = service.createTask("Overdue IP", Priority::HIGH, "", "2020-06-15", "");
    REQUIRE(ip.has_value());
    service.startTask(ip->get_id());
    // Not overdue — future due date
    service.createTask("Future task", Priority::LOW, "", "2099-12-31", "");
    // Not overdue — no due date
    service.createTask("No date task", Priority::MEDIUM, "", "", "");

    auto overdue = service.getOverdueTasks();
    REQUIRE(overdue.size() == 2);
    // Both overdue tasks should be present
    CHECK(overdue[0].get_title() == "Overdue TODO");
    CHECK(overdue[1].get_title() == "Overdue IP");
}

TEST_CASE("getOverdueTasks excludes DONE tasks even with past due dates", "[list][overdue]") {
    TaskStore store;
    TaskService service(store);

    auto task = service.createTask("Done past due", Priority::MEDIUM, "", "2020-01-01", "");
    REQUIRE(task.has_value());
    REQUIRE(service.completeTask(task->get_id()) == TransitionResult::Success);

    auto overdue = service.getOverdueTasks();
    CHECK(overdue.empty());
}

TEST_CASE("getOverdueTasks excludes CANCELLED tasks even with past due dates", "[list][overdue]") {
    TaskStore store;
    TaskService service(store);

    auto task = service.createTask("Cancelled past due", Priority::HIGH, "", "2020-01-01", "");
    REQUIRE(task.has_value());
    REQUIRE(service.cancelTask(task->get_id()) == TransitionResult::Success);

    auto overdue = service.getOverdueTasks();
    CHECK(overdue.empty());
}

TEST_CASE("getOverdueTasks excludes tasks with no due date", "[list][overdue]") {
    TaskStore store;
    TaskService service(store);

    service.createTask("No date", Priority::MEDIUM, "", "", "");
    service.createTask("Also no date", Priority::HIGH, "", "", "");

    auto overdue = service.getOverdueTasks();
    CHECK(overdue.empty());
}

TEST_CASE("getOverdueTasks returns empty vector when no tasks are overdue", "[list][overdue]") {
    TaskStore store;
    TaskService service(store);

    // All tasks have future due dates
    service.createTask("Future 1", Priority::LOW, "", "2099-01-01", "");
    service.createTask("Future 2", Priority::HIGH, "", "2099-12-31", "");

    auto overdue = service.getOverdueTasks();
    CHECK(overdue.empty());
}

// =============================================================================
// View & List Tasks — getSummary
// =============================================================================

TEST_CASE("getSummary returns correct counts across all statuses", "[list][summary]") {
    TaskStore store;
    TaskService service(store);

    // 2 TODO tasks (one overdue)
    service.createTask("TODO 1", Priority::LOW, "", "", "");
    service.createTask("TODO overdue", Priority::MEDIUM, "", "2020-01-01", "");

    // 1 IN_PROGRESS task (overdue)
    auto ip = service.createTask("IP overdue", Priority::HIGH, "", "2020-06-15", "");
    REQUIRE(ip.has_value());
    service.startTask(ip->get_id());

    // 1 DONE task (past due date, but NOT overdue because terminal)
    auto done = service.createTask("Done", Priority::MEDIUM, "", "2020-03-01", "");
    REQUIRE(done.has_value());
    service.completeTask(done->get_id());

    // 1 CANCELLED task
    auto cancelled = service.createTask("Cancelled", Priority::LOW, "", "", "");
    REQUIRE(cancelled.has_value());
    service.cancelTask(cancelled->get_id());

    auto summary = service.getSummary();
    CHECK(summary.total == 5);
    CHECK(summary.todo == 2);
    CHECK(summary.in_progress == 1);
    CHECK(summary.done == 1);
    CHECK(summary.cancelled == 1);
    CHECK(summary.overdue == 2);  // TODO overdue + IP overdue; DONE with past date is NOT overdue
}

TEST_CASE("getSummary returns all-zero struct when store is empty", "[list][summary]") {
    TaskStore store;
    TaskService service(store);

    auto summary = service.getSummary();
    CHECK(summary.total == 0);
    CHECK(summary.todo == 0);
    CHECK(summary.in_progress == 0);
    CHECK(summary.done == 0);
    CHECK(summary.cancelled == 0);
    CHECK(summary.overdue == 0);
}

// =============================================================================
// View & List Tasks — getTasksSortedByPriority
// Ported from: TaskServiceTest.getTasksSortedByPriority (Java)
// =============================================================================

TEST_CASE("getTasksSortedByPriority returns tasks sorted CRITICAL first", "[list][sort]") {
    TaskStore store;
    TaskService service(store);

    // Create tasks in mixed priority order
    service.createTask("Low task",      Priority::LOW,      "", "", "");
    service.createTask("High task",     Priority::HIGH,     "", "", "");
    service.createTask("Critical task", Priority::CRITICAL, "", "", "");
    service.createTask("Medium task",   Priority::MEDIUM,   "", "", "");

    auto sorted = service.getTasksSortedByPriority();
    REQUIRE(sorted.size() == 4);

    CHECK(sorted[0].get_priority() == Priority::CRITICAL);
    CHECK(sorted[0].get_title() == "Critical task");

    CHECK(sorted[1].get_priority() == Priority::HIGH);
    CHECK(sorted[1].get_title() == "High task");

    CHECK(sorted[2].get_priority() == Priority::MEDIUM);
    CHECK(sorted[2].get_title() == "Medium task");

    CHECK(sorted[3].get_priority() == Priority::LOW);
    CHECK(sorted[3].get_title() == "Low task");
}

TEST_CASE("getTasksSortedByPriority returns empty vector when no tasks exist", "[list][sort]") {
    TaskStore store;
    TaskService service(store);

    auto sorted = service.getTasksSortedByPriority();
    CHECK(sorted.empty());
}

// =============================================================================
// View & List Tasks — getOverdueTasks
// Ported from: TaskServiceTest.getOverdueTasks (Java)
// =============================================================================

TEST_CASE("getOverdueTasks returns only overdue tasks", "[list][overdue]") {
    TaskStore store;
    TaskService service(store);

    // Overdue TODO task (past due date, non-terminal status)
    service.createTask("Overdue TODO", Priority::MEDIUM, "", "2020-01-01", "");

    // Overdue IN_PROGRESS task
    auto ip = service.createTask("Overdue IP", Priority::HIGH, "", "2020-06-15", "");
    REQUIRE(ip.has_value());
    service.startTask(ip->get_id());

    // Not overdue — future due date
    service.createTask("Future task", Priority::LOW, "", "2099-12-31", "");

    // Not overdue — no due date
    service.createTask("No date task", Priority::MEDIUM, "", "", "");

    auto overdue = service.getOverdueTasks();
    REQUIRE(overdue.size() == 2);

    // Both overdue tasks should be present
    CHECK(overdue[0].get_title() == "Overdue TODO");
    CHECK(overdue[1].get_title() == "Overdue IP");
}

TEST_CASE("getOverdueTasks excludes DONE and CANCELLED tasks with past due dates", "[list][overdue]") {
    TaskStore store;
    TaskService service(store);

    // Create a task with past due date, then complete it
    auto done_task = service.createTask("Done task", Priority::MEDIUM, "", "2020-01-01", "");
    REQUIRE(done_task.has_value());
    service.completeTask(done_task->get_id());

    // Create a task with past due date, then cancel it
    auto cancelled_task = service.createTask("Cancelled task", Priority::MEDIUM, "", "2020-01-01", "");
    REQUIRE(cancelled_task.has_value());
    service.cancelTask(cancelled_task->get_id());

    auto overdue = service.getOverdueTasks();
    CHECK(overdue.empty());
}

TEST_CASE("getOverdueTasks returns empty vector when no tasks are overdue", "[list][overdue]") {
    TaskStore store;
    TaskService service(store);

    // Only future-dated and no-date tasks
    service.createTask("Future", Priority::HIGH, "", "2099-12-31", "");
    service.createTask("No date", Priority::LOW, "", "", "");

    auto overdue = service.getOverdueTasks();
    CHECK(overdue.empty());
}

TEST_CASE("getOverdueTasks returns empty vector when store is empty", "[list][overdue]") {
    TaskStore store;
    TaskService service(store);

    auto overdue = service.getOverdueTasks();
    CHECK(overdue.empty());
}

// =============================================================================
// View & List Tasks — getSummary
// Ported from: TaskServiceTest.getSummary (Java)
// =============================================================================

TEST_CASE("getSummary returns correct counts across all statuses", "[list][summary]") {
    TaskStore store;
    TaskService service(store);

    // 2 TODO tasks (one overdue, one not)
    service.createTask("TODO 1", Priority::LOW, "", "2020-01-01", "");   // overdue
    service.createTask("TODO 2", Priority::MEDIUM, "", "2099-12-31", "");

    // 1 IN_PROGRESS task (overdue)
    auto ip = service.createTask("IP task", Priority::HIGH, "", "2020-06-15", "");
    REQUIRE(ip.has_value());
    service.startTask(ip->get_id());

    // 1 DONE task (past due date but NOT overdue since terminal)
    auto done = service.createTask("Done task", Priority::MEDIUM, "", "2020-03-01", "");
    REQUIRE(done.has_value());
    service.completeTask(done->get_id());

    // 1 CANCELLED task
    auto cancelled = service.createTask("Cancelled task", Priority::LOW, "", "", "");
    REQUIRE(cancelled.has_value());
    service.cancelTask(cancelled->get_id());

    auto summary = service.getSummary();

    CHECK(summary.total == 5);
    CHECK(summary.todo == 2);
    CHECK(summary.in_progress == 1);
    CHECK(summary.done == 1);
    CHECK(summary.cancelled == 1);
    CHECK(summary.overdue == 2);  // TODO 1 + IP task
}

TEST_CASE("getSummary returns all-zero struct when store is empty", "[list][summary]") {
    TaskStore store;
    TaskService service(store);

    auto summary = service.getSummary();

    CHECK(summary.total == 0);
    CHECK(summary.todo == 0);
    CHECK(summary.in_progress == 0);
    CHECK(summary.done == 0);
    CHECK(summary.cancelled == 0);
    CHECK(summary.overdue == 0);
}
