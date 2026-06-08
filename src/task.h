#ifndef TASK_H
#define TASK_H

#include <string>

#include "task_status.h"
#include "priority.h"

/// Minimal Task model — sufficient for TaskStore operations.
/// Full behavior (status transitions, overdue logic, validation) will be
/// added in later stories.
class Task {
public:
    Task(int id, const std::string& title);

    // --- Getters (const) ---
    int id() const { return id_; }
    const std::string& title() const { return title_; }
    const std::string& description() const { return description_; }
    TaskStatus status() const { return status_; }
    Priority priority() const { return priority_; }
    const std::string& due_date() const { return due_date_; }
    const std::string& assignee() const { return assignee_; }
    const std::string& created_date() const { return created_date_; }

    // --- Setters (minimal set needed for testing TaskStore filters) ---
    void set_status(TaskStatus status) { status_ = status; }
    void set_priority(Priority priority) { priority_ = priority; }
    void set_assignee(const std::string& assignee) { assignee_ = assignee; }
    void set_due_date(const std::string& due_date) { due_date_ = due_date; }
    void set_title(const std::string& title) { title_ = title; }
    void set_description(const std::string& description) { description_ = description; }

private:
    int id_;
    std::string title_;
    std::string description_;
    TaskStatus status_;
    Priority priority_;
    std::string due_date_;
    std::string assignee_;
    std::string created_date_;
};

#endif // TASK_H
