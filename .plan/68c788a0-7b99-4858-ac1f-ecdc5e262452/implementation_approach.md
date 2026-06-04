# Implementation Approach

## Overdue Task Identification — Implementation Approach

### Core Pattern: Match Java's `Task.isOverdue()` exactly

The overdue check lives on the **`Task` model** as a method, matching the Java source:

```cpp
// task.h
bool is_overdue() const;
```

```cpp
// task.cpp
bool Task::is_overdue() const {
    return !due_date_.empty() && get_today_string() > due_date_ && !is_terminal(status_);
}
```

### Date Handling

Per the locked technology stack decision, dates are stored as `std::string` in `YYYY-MM-DD` format and compared lexicographically.

**Getting "today"**: A free function `get_today_string()` is called each time `is_overdue()` is invoked — a direct equivalent of Java's `LocalDate.now()` inside the method. This function uses C++17's `<ctime>` / `<chrono>` to get the current system date and format it as `YYYY-MM-DD`.

```cpp
// date_utils.h (header-only utility)
#include <string>
#include <ctime>

inline std::string get_today_string() {
    auto now = std::time(nullptr);
    auto* tm = std::localtime(&now);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", tm);
    return std::string(buf);
}
```

### Terminal Status Check

A free function `is_terminal()` replaces Java's `TaskStatus.isTerminal()` method:

```cpp
// task_status.h
enum class TaskStatus { TODO, IN_PROGRESS, DONE, CANCELLED };

inline bool is_terminal(TaskStatus s) {
    return s == TaskStatus::DONE || s == TaskStatus::CANCELLED;
}
```

Overdue logic: a task is overdue when **all three** conditions hold:
1. `due_date` is non-empty (task has a due date)
2. Today's date string is lexicographically greater than `due_date` (due date is in the past)
3. Status is **not** terminal (`DONE` or `CANCELLED`)

### Service Layer: `getOverdueTasks()`

`TaskService::get_overdue_tasks()` filters all tasks using `Task::is_overdue()`, matching Java's stream-filter pattern:

```cpp
// task_service.cpp
std::vector<Task> TaskService::get_overdue_tasks() const {
    auto all = store_.find_all();
    std::vector<Task> result;
    std::copy_if(all.begin(), all.end(), std::back_inserter(result),
                 [](const Task& t) { return t.is_overdue(); });
    return result;
}
```

### Summary Integration

`TaskService::get_summary()` counts overdue tasks using the same `is_overdue()` method, matching how Java's `getSummary()` includes an overdue count in `TaskSummary`.

### CLI Integration

Menu option **10 ("Show overdue tasks")** calls `TaskService::get_overdue_tasks()` and displays the results, matching the Java CLI's existing behavior.

### Test Porting

The Java test `getOverdueTasks_returnsOnlyOverdue` maps directly:
- Creates a task with due date = yesterday (`get_today_string()` minus 1 day — will need a small date arithmetic helper or hardcoded past date string in the test)
- Creates a task with due date = future
- Creates a task with no due date
- Asserts only the first task appears in the overdue list

Additional tests from acceptance criteria are already covered by the existing Java test pattern:
- AC1 (overdue with TODO/IN_PROGRESS) → covered by the existing test
- AC2 (DONE/CANCELLED not overdue) → covered by `is_terminal()` check
- AC3 (no due date → never overdue) → covered by the existing test's "NoDue" task
- AC4 (filter to overdue only) → covered by `get_overdue_tasks()` returning only overdue tasks

### File Placement

| File | Purpose |
|---|---|
| `date_utils.h` | Header-only `get_today_string()` utility |
| `task_status.h` | `enum class TaskStatus` + `is_terminal()` (already needed) |
| `task.h / task.cpp` | `Task::is_overdue()` method |
| `task_service.h / task_service.cpp` | `get_overdue_tasks()` + overdue count in `get_summary()` |
| `task_service_test.cpp` | Ported overdue test |
