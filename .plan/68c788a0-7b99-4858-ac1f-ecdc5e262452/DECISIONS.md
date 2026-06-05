# Locked Decisions for Story 68c788a0-7b99-4858-ac1f-ecdc5e262452

## Implementation Approach
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

## Validation
## Validation & Edge Cases for Overdue Detection

### Overdue Logic Validation Rules

The `Task::is_overdue()` method enforces three conditions. Each maps to an acceptance criterion:

| Condition | Rule | AC |
|---|---|---|
| Due date must exist | Empty string `due_date_` → **never overdue** | AC3 |
| Due date must be in the past | `get_today_string() > due_date_` (strict greater-than, so tasks due **today** are NOT overdue) | AC1 |
| Status must be non-terminal | `!is_terminal(status_)` — only `TODO` and `IN_PROGRESS` tasks can be overdue | AC1, AC2 |

### Edge Cases

1. **Task due today**: A task with `due_date == today` is **not** overdue (matches Java's `LocalDate.now().isAfter(dueDate)` which is strictly after, not on-or-after). The `>` operator on strings gives the same semantics.

2. **Task completed/cancelled after becoming overdue**: Once a task transitions to `DONE` or `CANCELLED`, it is no longer overdue regardless of due date. No special handling needed — `is_terminal()` covers this.

3. **Task with empty vs. missing due date**: Since C++17 dates are stored as `std::string`, an unset due date is represented as an empty string `""`. The check `!due_date_.empty()` handles this. There is no separate "null" concept.

4. **Invalid date strings**: Per the locked migration strategy, date input validation happens at the CLI layer when the user enters a due date. The `is_overdue()` method assumes `due_date_` is either empty or a valid `YYYY-MM-DD` string — no defensive parsing inside the overdue check itself.

### Date Format Validation (CLI Input Layer)

When the user sets a due date via the CLI (menu option 1 — Add task), the input is validated before storing:

- Must match `YYYY-MM-DD` format
- Basic range validation: month 01-12, day 01-31
- Invalid input prompts re-entry with a clear error message
- This validation lives in the CLI input handling code in `main.cpp`, not in `Task` or `TaskService`

### No Business Rule Surprises

- **No grace periods**: Overdue is strictly "past the due date"
- **No timezone handling**: Uses local system time via `std::localtime()`, matching Java's `LocalDate.now()` behavior
- **No notifications or side effects**: Overdue is a pure query — checking `is_overdue()` does not modify the task in any way
