# Implementation Approach

## TaskStore Implementation

### Container & Storage
- **`std::map<int, Task>`** as the backing store — `std::map` is an ordered container that preserves insertion order by key, which for monotonically increasing IDs is equivalent to insertion order. This directly replaces Java's `LinkedHashMap<Integer, Task>`.
- Tasks stored **by value** (RAII, no heap allocation), consistent with the locked migration strategy.

### Sequential ID Generation
- **`int next_id_`** member initialized to `1`, incremented on each call to `next_id()`.
- Simple `return next_id_++;` — no `std::atomic` needed since the locked NFR confirms single-threaded CLI usage.
- IDs are never reused: the counter only increments, regardless of deletions (satisfies AC #3).

### API Surface (C++ signatures)

```cpp
class TaskStore {
public:
    Task& save(const Task& task);          // Overwrites silently if ID exists
    std::optional<std::reference_wrapper<Task>> find_by_id(int id);
    std::vector<Task> find_all() const;    // Returns copies (like Java's new ArrayList<>)
    std::vector<Task> find_by_status(TaskStatus status) const;
    std::vector<Task> find_by_priority(Priority priority) const;
    std::vector<Task> find_by_assignee(const std::string& assignee) const;
    bool remove(int id);                   // Returns false if ID not found
    int next_id();                         // Monotonically increasing, never reused

private:
    std::map<int, Task> store_;
    int next_id_ = 1;
};
```

### Key Design Choices

| Choice | Rationale |
|---|---|
| `save()` returns `Task&` | Allows caller to use the stored reference directly, mirrors Java's `return task` after `put()` |
| `find_by_id()` returns `std::optional<std::reference_wrapper<Task>>` | Enables in-place mutation (e.g., `startProgress()`) without a second `save()` call — matches how Java works with object references |
| `find_all()` returns `std::vector<Task>` (copies) | Matches Java's `new ArrayList<>(store.values())` — callers get a snapshot, not a live view |
| `remove()` returns `bool` | Direct match for Java's `store.remove(id) != null` |
| `std::map` over `std::unordered_map` | Guarantees iteration in key (insertion) order for sequential IDs, preserving AC #2 |

### Java → C++ Method Mapping

| Java (`InMemoryTaskRepository`) | C++ (`TaskStore`) | Notes |
|---|---|---|
| `save(Task task)` | `save(const Task& task)` | Copies task into map |
| `findById(int id)` | `find_by_id(int id)` | Returns optional reference |
| `findAll()` | `find_all()` | Returns vector of copies |
| `findByStatus(TaskStatus)` | `find_by_status(TaskStatus)` | Linear scan with `std::copy_if` |
| `findByPriority(Priority)` | `find_by_priority(Priority)` | Linear scan with `std::copy_if` |
| `findByAssignee(String)` | `find_by_assignee(const std::string&)` | Linear scan with `std::copy_if` |
| `delete(int id)` | `remove(int id)` | `delete` is a C++ keyword |
| `nextId()` | `next_id()` | Simple int counter |

### Files Produced
- `task_store.h` — class declaration
- `task_store.cpp` — method implementations
