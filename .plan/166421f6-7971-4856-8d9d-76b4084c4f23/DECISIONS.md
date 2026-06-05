# Locked Decisions for Story 166421f6-7971-4856-8d9d-76b4084c4f23

## Implementation Approach
## Implementation Approach for Task Summary Dashboard

### Overview
Implement `TaskService::getSummary()` in C++ to return a `TaskSummary` struct with counts for total, TODO, IN_PROGRESS, DONE, CANCELLED, and overdue tasks. This directly ports Java's `TaskService.getSummary()` with an idiomatic C++ improvement: a **single-pass iteration** instead of Java's 6 separate stream passes.

### Key Technical Decisions

**1. Single-pass iteration (improves on Java)**
The Java source iterates the task list 6 times (once per counter via separate `.stream().filter().count()` calls). The C++ version will use a single `for` loop over `TaskStore::findAll()`, incrementing all counters in one pass using a `switch` on `task.getStatus()` plus an `isOverdue()` check.

```cpp
TaskSummary TaskService::getSummary() const {
    const auto& tasks = store_.findAll();
    int total = 0, todo = 0, in_progress = 0, done = 0, cancelled = 0, overdue = 0;
    for (const auto& [id, task] : tasks) {
        ++total;
        switch (task.getStatus()) {
            case TaskStatus::TODO:        ++todo; break;
            case TaskStatus::IN_PROGRESS: ++in_progress; break;
            case TaskStatus::DONE:        ++done; break;
            case TaskStatus::CANCELLED:   ++cancelled; break;
        }
        if (task.isOverdue()) ++overdue;
    }
    return {total, todo, in_progress, done, cancelled, overdue};
}
```

**2. `TaskSummary` as a header-only struct (per architecture decision)**
```cpp
// task_summary.h
struct TaskSummary {
    int total = 0;
    int todo = 0;
    int in_progress = 0;
    int done = 0;
    int cancelled = 0;
    int overdue = 0;
    
    std::string to_string() const; // replaces Java's toString()
};
```
- Uses `int` (not `long`) since the NFR caps scale at ~100 tasks
- Default member initializers to zero for safety
- `to_string()` method replicates Java's `String.format(...)` output

**3. `Task::isOverdue()` method**
Ports Java's `isOverdue()` logic. A task is overdue when:
- `due_date` is non-empty AND
- `due_date < today_date()` (lexicographic comparison of YYYY-MM-DD strings) AND
- status is NOT terminal (`DONE` or `CANCELLED`)

```cpp
bool Task::isOverdue() const {
    return !due_date_.empty() 
        && due_date_ < today_date() 
        && !isTerminal();
}
```

**4. `today_date()` helper utility function**
A free function in a small utility header (`date_utils.h`) that returns today's date as a `std::string` in `YYYY-MM-DD` format, using `std::chrono::system_clock` and `std::time_t` / `std::strftime`:

```cpp
// date_utils.h
#pragma once
#include <string>
std::string today_date();
```

**5. `TaskStatus::isTerminal()` as a free function**
Java's `TaskStatus.isTerminal()` method becomes a free function since C++ `enum class` can't have methods:
```cpp
inline bool isTerminal(TaskStatus s) {
    return s == TaskStatus::DONE || s == TaskStatus::CANCELLED;
}
```

**6. Real-time counts (AC #4)**
Since `getSummary()` iterates live data from `TaskStore` on every call (no caching), counts always reflect the current state — satisfying AC #4 with no extra work.

### Files Involved
| File | Action | Purpose |
|---|---|---|
| `task_summary.h` | Create | `TaskSummary` struct with `to_string()` |
| `task_status.h` | Create | `TaskStatus` enum class + `isTerminal()` helper |
| `date_utils.h / date_utils.cpp` | Create | `today_date()` utility |
| `task.h / task.cpp` | Create/Modify | `Task` class with `isOverdue()` method |
| `task_store.h / task_store.cpp` | Create/Modify | Storage with `findAll()` returning all tasks |
| `task_service.h / task_service.cpp` | Create/Modify | `getSummary()` implementation |
| `task_service_test.cpp` | Create/Modify | Port `getSummary_countsCorrectly` test |

### Acceptance Criteria Mapping
- **AC1** (counts by status + overdue): Single-pass loop covers all counters
- **AC2** (overdue = past due + non-terminal): `Task::isOverdue()` checks `!isTerminal()`
- **AC3** (DONE/CANCELLED not overdue): `isTerminal()` guard excludes them
- **AC4** (real-time): No caching — computed fresh on each call

## Validation
## Validation & Edge Cases for Task Summary

### Business Rules (from Acceptance Criteria)

**Overdue definition (AC #2 and #3):**
A task is overdue if and only if ALL three conditions are true:
1. `due_date` is set (non-empty string)
2. `due_date` is strictly before today (`due_date < today_date()`)
3. Status is NOT terminal — i.e., status is `TODO` or `IN_PROGRESS`

Tasks with status `DONE` or `CANCELLED` are **never** counted as overdue, regardless of due date.

### Edge Cases Handled

| Scenario | Expected Behavior |
|---|---|
| No tasks exist | All counts return 0 (zero-initialized struct) |
| Task has no due date (`due_date` empty) | Not counted as overdue |
| Task due today | **Not overdue** — overdue requires strictly past (`<`, not `<=`), matching Java's `isAfter()` semantics |
| Task due yesterday, status TODO | Counted as overdue |
| Task due yesterday, status DONE | NOT overdue (terminal status) |
| Task due yesterday, status CANCELLED | NOT overdue (terminal status) |
| Task due yesterday, status IN_PROGRESS | Counted as overdue |
| All tasks in same status | That status count equals `total`, others are 0 |
| Task added then immediately deleted | Not reflected in summary (deleted from store) |

### Input Validation
- `getSummary()` takes no input parameters — no input validation needed
- The method is purely a read operation over the current store contents
- No error states possible: an empty store simply returns all-zero `TaskSummary`

### Date String Validation
- `today_date()` always produces a valid `YYYY-MM-DD` string (system clock formatted via `strftime`)
- Task due dates are validated at input time (when creating/updating tasks, not in this story's scope)
- Lexicographic comparison of `YYYY-MM-DD` strings is correct for chronological ordering

### Consistency with Java Behavior
- Java uses `LocalDate.now().isAfter(dueDate)` — this is a **strict** comparison (today == due date → NOT overdue)
- C++ equivalent: `due_date_ < today_date()` — lexicographic `<` preserves the same strict semantics
- The `getSummary_countsCorrectly` test from Java will be ported, validating status counts (the test doesn't cover overdue specifically, but `getOverdueTasks_returnsOnlyOverdue` validates the `isOverdue()` logic used by summary)
