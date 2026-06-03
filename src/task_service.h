#ifndef TASK_SERVICE_H
#define TASK_SERVICE_H

#include "task.h"
#include "task_store.h"
#include "priority.h"
#include <optional>
#include <string>
#include <vector>

class TaskService {
public:
    explicit TaskService(TaskStore& store);

    // --- Create (primary focus of this story) ---
    std::optional<Task> createTask(
        const std::string& title,
        Priority priority = Priority::MEDIUM,
        const std::string& description = "",
        const std::string& due_date = "",
        const std::string& assignee = ""
    );

    // --- Read ---
    std::optional<Task> getTask(int id) const;
    std::vector<Task> getAllTasks() const;
    std::vector<Task> getPendingTasks() const;

    // --- Update ---
    bool startTask(int id);
    bool completeTask(int id);
    bool cancelTask(int id);
    bool assignTask(int id, const std::string& assignee);
    bool updatePriority(int id, Priority priority);

    // --- Delete ---
    bool deleteTask(int id);

private:
    TaskStore& store_;
};

#endif // TASK_SERVICE_H
