#include "task_store.h"

TaskStore::TaskStore() : next_id_(1) {}

Task TaskStore::save(const Task& task) {
    tasks_.insert_or_assign(task.get_id(), task);
    return tasks_.at(task.get_id());
}

std::optional<Task> TaskStore::find_by_id(int id) const {
    auto it = tasks_.find(id);
    if (it != tasks_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<Task> TaskStore::find_all() const {
    std::vector<Task> result;
    result.reserve(tasks_.size());
    for (const auto& [id, task] : tasks_) {
        result.push_back(task);
    }
    return result;
}

bool TaskStore::delete_task(int id) {
    return tasks_.erase(id) > 0;
}

int TaskStore::next_id() {
    return next_id_++;
}
