#ifndef TASK_STORE_H
#define TASK_STORE_H

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "priority.h"
#include "task.h"
#include "task_status.h"

/// In-memory task storage with sequential ID generation.
///
/// Replaces Java's InMemoryTaskRepository. Uses std::map<int, Task> as the
/// backing store — ordered by key, which for monotonically increasing IDs
/// preserves insertion order. Tasks are stored by value (RAII).
///
/// Thread safety: none (single-threaded CLI usage per locked NFR).
class TaskStore {
public:
    /// Stores a copy of the task, keyed by task.id().
    /// If a task with the same ID already exists, it is silently overwritten.
    /// Returns a reference to the stored task.
    Task& save(const Task& task);

    /// Looks up a task by ID.
    /// Returns std::nullopt if the ID is not found.
    /// The returned reference allows in-place mutation of the stored task.
    std::optional<std::reference_wrapper<Task>> find_by_id(int id);

    /// Returns copies of all stored tasks, in ID (insertion) order.
    std::vector<Task> find_all() const;

    /// Returns copies of tasks matching the given status.
    std::vector<Task> find_by_status(TaskStatus status) const;

    /// Returns copies of tasks matching the given priority.
    std::vector<Task> find_by_priority(Priority priority) const;

    /// Returns copies of tasks assigned to the given assignee.
    std::vector<Task> find_by_assignee(const std::string& assignee) const;

    /// Removes the task with the given ID.
    /// Returns true if a task was removed, false if the ID was not found.
    bool remove(int id);

    /// Returns the next unique ID and increments the internal counter.
    /// IDs are monotonically increasing and never reused.
    int next_id();

private:
    std::map<int, Task> store_;
    int next_id_ = 1;
};

#endif // TASK_STORE_H
