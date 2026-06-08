#include "task_store.h"

Task& TaskStore::save(const Task& task) {
    // insert_or_assign works with non-default-constructible types (C++17).
    // If the key already exists the value is overwritten; otherwise a new
    // entry is inserted.  Mirrors Java's LinkedHashMap.put().
    auto [it, _] = store_.insert_or_assign(task.id(), task);
    return it->second;
}

std::optional<std::reference_wrapper<Task>> TaskStore::find_by_id(int id) {
    auto it = store_.find(id);
    if (it == store_.end()) {
        return std::nullopt;
    }
    return std::ref(it->second);
}

std::vector<Task> TaskStore::find_all() const {
    // Returns copies — matches Java's `new ArrayList<>(store.values())`.
    std::vector<Task> result;
    result.reserve(store_.size());
    for (const auto& [key, task] : store_) {
        result.push_back(task);
    }
    return result;
}

std::vector<Task> TaskStore::find_by_status(TaskStatus status) const {
    std::vector<Task> result;
    for (const auto& [key, task] : store_) {
        if (task.status() == status) {
            result.push_back(task);
        }
    }
    return result;
}

std::vector<Task> TaskStore::find_by_priority(Priority priority) const {
    std::vector<Task> result;
    for (const auto& [key, task] : store_) {
        if (task.priority() == priority) {
            result.push_back(task);
        }
    }
    return result;
}

std::vector<Task> TaskStore::find_by_assignee(const std::string& assignee) const {
    std::vector<Task> result;
    for (const auto& [key, task] : store_) {
        if (task.assignee() == assignee) {
            result.push_back(task);
        }
    }
    return result;
}

bool TaskStore::remove(int id) {
    return store_.erase(id) > 0;
}

int TaskStore::next_id() {
    return next_id_++;
}
