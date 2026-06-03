#include "task_service.h"
#include "date_utils.h"
#include <iostream>
#include <algorithm>

TaskService::TaskService(TaskStore& store) : store_(store) {}

// =============================================================================
// Create — primary method for the "Create a New Task" story.
// All creation-related business rules are centralized here.
// =============================================================================
std::optional<Task> TaskService::createTask(
    const std::string& title,
    Priority priority,
    const std::string& description,
    const std::string& due_date,
    const std::string& assignee)
{
    // Validate title: must be non-empty after trimming whitespace
    std::string trimmed = title;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
    if (trimmed.empty()) {
        std::cerr << "Title cannot be empty." << std::endl;
        return std::nullopt;
    }

    // Validate due date if provided
    if (!due_date.empty() && !is_valid_date(due_date)) {
        std::cerr << "Invalid date format — due date not set." << std::endl;
        return std::nullopt;
    }

    // Build the task
    int id = store_.next_id();
    Task task(id, title, today_as_string());
    task.set_priority(priority);
    task.set_description(description);
    task.set_due_date(due_date);
    task.set_assignee(assignee);

    return store_.save(task);
}

// =============================================================================
// Read
// =============================================================================
std::optional<Task> TaskService::getTask(int id) const {
    return store_.find_by_id(id);
}

std::vector<Task> TaskService::getAllTasks() const {
    return store_.find_all();
}

std::vector<Task> TaskService::getPendingTasks() const {
    return store_.find_by_status(TaskStatus::TODO);
}

// =============================================================================
// Update — state transitions via the store's update_status helper
// =============================================================================
TransitionResult TaskService::startTask(int id) {
    return store_.update_status(id, [](Task& t) { return t.start_progress(); });
}

TransitionResult TaskService::completeTask(int id) {
    return store_.update_status(id, [](Task& t) { return t.complete(); });
}

TransitionResult TaskService::cancelTask(int id) {
    return store_.update_status(id, [](Task& t) { return t.cancel(); });
}

bool TaskService::assignTask(int id, const std::string& assignee) {
    return store_.update(id, [&assignee](Task& t) { t.set_assignee(assignee); });
}

bool TaskService::updatePriority(int id, Priority priority) {
    return store_.update(id, [priority](Task& t) { t.set_priority(priority); });
}

// =============================================================================
// Delete
// =============================================================================
bool TaskService::deleteTask(int id) {
    return store_.delete_task(id);
}
