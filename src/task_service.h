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

    std::optional<Task> createTask(
        const std::string& title,
        Priority priority = Priority::MEDIUM,
        const std::string& description = "",
        const std::string& due_date = "",
        const std::string& assignee = ""
    );

private:
    TaskStore& store_;
};

#endif // TASK_SERVICE_H
