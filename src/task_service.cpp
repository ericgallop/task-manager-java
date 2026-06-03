#include "task_service.h"
#include "date_utils.h"
#include <iostream>
#include <algorithm>

TaskService::TaskService(TaskStore& store) : store_(store) {}

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
