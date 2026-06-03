#ifndef TASK_H
#define TASK_H

#include <string>
#include "priority.h"
#include "task_status.h"

class Task {
public:
    Task(int id, const std::string& title, const std::string& created_date);

    // Getters
    int get_id() const;
    const std::string& get_title() const;
    const std::string& get_description() const;
    Priority get_priority() const;
    TaskStatus get_status() const;
    const std::string& get_assignee() const;
    const std::string& get_due_date() const;
    const std::string& get_created_date() const;

    // Setters
    void set_title(const std::string& title);
    void set_description(const std::string& description);
    void set_priority(Priority priority);
    void set_status(TaskStatus status);
    void set_assignee(const std::string& assignee);
    void set_due_date(const std::string& due_date);

    // Status helpers
    bool is_completed() const;
    bool is_overdue() const;
    void complete();
    void cancel();
    void start_progress();

    std::string to_string() const;

private:
    int id_;
    std::string title_;
    std::string description_;
    Priority priority_;
    TaskStatus status_;
    std::string assignee_;
    std::string due_date_;
    std::string created_date_;
};

#endif // TASK_H
