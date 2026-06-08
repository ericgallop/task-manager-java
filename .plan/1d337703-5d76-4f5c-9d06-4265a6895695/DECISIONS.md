# Locked Decisions for Story 1d337703-5d76-4f5c-9d06-4265a6895695

## Implementation Approach
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

## Validation
## Validation & Error Handling for TaskStore

This story's scope is the storage layer only. TaskStore is a thin data container — validation is minimal and follows the Java original's behavior.

### TaskStore Error Handling

| Method | Error Scenario | Behavior | Rationale |
|---|---|---|---|
| `save(task)` | ID already exists | Overwrite silently | Matches Java `LinkedHashMap.put()` — used for both insert and implicit update |
| `save(task)` | Any task passed in | Always succeeds | No field validation at storage layer (Java original has none either) |
| `find_by_id(id)` | ID not found | Returns `std::nullopt` | Idiomatic C++17, matches Java's `Optional.empty()` |
| `remove(id)` | ID not found | Returns `false` | Matches Java's `store.remove(id) != null` |
| `next_id()` | Counter overflow | No guard | At ≤100 tasks (per NFR), `int` overflow is not a practical concern |
| `find_all()` / `find_by_*()` | Empty store | Returns empty `std::vector` | Standard behavior, no special handling |

### What TaskStore Does NOT Validate
- **Task field validity** (empty title, null priority, etc.) — this is the responsibility of `Task` itself (Java's `Task.setTitle()` throws `IllegalArgumentException` on blank titles; the C++ equivalent will be handled in the Task model story)
- **Status transition rules** — enforced in `Task.startProgress()`, `Task.complete()`, etc., not in the store
- **Duplicate titles or content** — not validated in Java, not validated in C++

### Edge Cases Addressed by Design
1. **Deleted ID reuse** — IDs never reuse because `next_id_` only increments (AC #3)
2. **Insertion order preservation** — `std::map<int, Task>` with sequential keys guarantees ordered iteration (AC #2)
3. **Session-scoped ephemeral data** — `TaskStore` is a stack-allocated object in `main()`; process exit destroys all data (AC #4)

### No Exceptions Thrown
TaskStore itself throws no exceptions. All error conditions are communicated via return types (`bool`, `std::optional`). This is consistent with the locked migration strategy's principle of "stronger error handling — replace silent boolean returns with more descriptive feedback where practical" while keeping the storage layer simple — richer error feedback will be added at the service/CLI layers in later stories.
