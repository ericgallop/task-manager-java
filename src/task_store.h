#ifndef TASK_STORE_H
#define TASK_STORE_H

#include "task.h"
#include <map>
#include <optional>
#include <vector>

class TaskStore {
public:
    TaskStore();

    Task save(const Task& task);
    std::optional<Task> find_by_id(int id) const;
    std::vector<Task> find_all() const;
    bool delete_task(int id);
    int next_id();

private:
    std::map<int, Task> tasks_;
    int next_id_;
};

#endif // TASK_STORE_H
