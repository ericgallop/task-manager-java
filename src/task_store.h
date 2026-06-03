#ifndef TASK_STORE_H
#define TASK_STORE_H

#include "task.h"
#include <map>
#include <optional>
#include <vector>
#include <functional>

class TaskStore {
public:
    TaskStore();

    // Core CRUD operations
    Task save(const Task& task);
    std::optional<Task> find_by_id(int id) const;
    std::vector<Task> find_all() const;
    bool delete_task(int id);
    int next_id();

    // Filtered queries
    std::vector<Task> find_by_status(TaskStatus status) const;
    std::vector<Task> find_by_priority(Priority priority) const;
    std::vector<Task> find_by_assignee(const std::string& assignee) const;

    // Update helper: find task by id, apply mutator, save back.
    // Returns true if task was found and updated, false otherwise.
    bool update(int id, std::function<void(Task&)> mutator);

    // Transition helper: find task by id, apply transition lambda.
    // Returns NotFound if id doesn't exist, otherwise returns
    // the TransitionResult from the transition lambda.
    TransitionResult update_status(int id, std::function<TransitionResult(Task&)> transition);

private:
    std::map<int, Task> tasks_;
    int next_id_;
};

#endif // TASK_STORE_H
