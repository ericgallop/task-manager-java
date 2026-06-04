# Locked Decisions for Story 8ce012f1-6d35-4a21-b1b9-5feaee05ffbb

## Implementation Approach
## Implementation Approach — View and List Tasks

### What Exists Today
The C++ project already has the full model layer (`Task`, `Priority`, `TaskStatus`), storage layer (`TaskStore`), and a partial service layer (`TaskService`) with CRUD, lifecycle transitions, assign, and priority update. The CLI (`main.cpp`) has menu options 1–8 and 0 (exit). Tests use **Catch2** (not Google Test as originally planned at the project level).

### What's Missing (Gaps to Fill)

#### 1. Service Layer — New Methods on `TaskService`
The Java `TaskService` has three methods not yet ported to C++. All three are needed by this story:

| Method | Purpose | Implementation |
|---|---|---|
| `getTasksSortedByPriority()` | Returns all tasks sorted CRITICAL → HIGH → MEDIUM → LOW | Copy `find_all()` result into a `std::vector`, then `std::sort` with a lambda comparing `get_priority()` in descending order |
| `getOverdueTasks()` | Returns only tasks where `is_overdue() == true` | `std::copy_if` over `find_all()` filtering on `task.is_overdue()` |
| `getSummary()` | Returns a `TaskSummary` struct with counts by status + overdue count | Single pass over `find_all()`, counting each status and overdue flag |

#### 2. New Header — `task_summary.h`
A simple header-only struct (per architecture decision):
```cpp
struct TaskSummary {
    int total = 0;
    int todo = 0;
    int in_progress = 0;
    int done = 0;
    int cancelled = 0;
    int overdue = 0;
};
```

#### 3. CLI Output — ANSI Color-Coded Task Display
The `listTasks()` function in `main.cpp` needs to be updated to:
- Call `getTasksSortedByPriority()` instead of `getAllTasks()` (always sorted by priority, CRITICAL first)
- Apply ANSI color codes per task status when printing:
  - **Overdue** (checked first, takes priority): yellow/amber text (`\033[33m`)
  - **DONE**: green text (`\033[32m`)
  - **CANCELLED**: dim/grey text (`\033[2m`)
  - **TODO / IN_PROGRESS**: default (no color, `\033[0m`)
- Reset color after each line (`\033[0m`)

The coloring logic lives in `main.cpp` (the UI layer), **not** in `Task::to_string()`. The `to_string()` method remains color-free so it can be used in non-display contexts (tests, logging). A small helper function like `print_task_colored(const Task&)` in `main.cpp` wraps `to_string()` with the appropriate ANSI escape prefix.

#### 4. CLI Menu — Add Options 9 and 10
Two missing menu items from the Java CLI:
- **9. Summary** — calls `service.getSummary()` and prints the formatted summary line
- **10. Show overdue** — calls `service.getOverdueTasks()` and prints each with ANSI yellow highlighting, or "No overdue tasks." if empty

#### 5. Empty State Handling
When no tasks exist (AC #2), the `listTasks()` function already prints `"No tasks."` — this is preserved as-is.

### Sort Order
The list always sorts by **priority descending** (CRITICAL → LOW), matching the Java CLI's `getTasksSortedByPriority()`. No user-selectable sort order — the Java GUI's sort dropdown is not migrated.

### Priority Sort Order
Java's `Priority` enum has a natural ordering where `compareTo` on `Priority.CRITICAL.ordinal()` gives the highest value. In C++, `enum class Priority { LOW, MEDIUM, HIGH, CRITICAL }` has the same property — casting to `int` gives `LOW=0, MEDIUM=1, HIGH=2, CRITICAL=3`. Sorting in **descending** order of this integer produces the correct CRITICAL-first ordering.

### Files Changed

| File | Change |
|---|---|
| `src/task_summary.h` | **New file** — header-only struct |
| `src/task_service.h` | Add `getTasksSortedByPriority()`, `getOverdueTasks()`, `getSummary()` declarations |
| `src/task_service.cpp` | Implement the three new methods |
| `src/main.cpp` | Update `listTasks()` to sort by priority + ANSI colors; add `print_task_colored()` helper; add menu options 9 (Summary) and 10 (Show overdue); update `printMenu()` |
| `tests/task_service_test.cpp` | Add tests for the three new service methods (sort order, overdue filtering, summary counts) |

### CMake
No changes needed — `task_summary.h` is header-only and the existing `task_lib` target already includes the source files that will be modified.

## Validation
## Validation & Edge Cases — View and List Tasks

This story is primarily a **read-only display** story. There is no user input to validate for the list/summary/overdue operations — the user just selects a menu option and sees output. Validation concerns are minimal but worth documenting.

### Edge Cases Handled

| Scenario | Behavior |
|---|---|
| **No tasks exist** (AC #2) | `listTasks()` prints `"No tasks."` — already implemented, preserved as-is |
| **No overdue tasks** (menu option 10) | Prints `"No overdue tasks."` — matches Java CLI behavior |
| **Summary with zero tasks** | All counts are 0: `Total: 0 | TODO: 0 | In Progress: 0 | Done: 0 | Cancelled: 0 | Overdue: 0` |
| **Task is both overdue AND has a status** | Overdue check takes display priority over status color (e.g., an overdue TODO renders in yellow, not default) |
| **Tasks with no due date** | `is_overdue()` already returns `false` when `due_date_` is empty — these never appear as overdue |
| **Terminal tasks (DONE/CANCELLED) with past due dates** | `is_overdue()` already returns `false` for terminal statuses — a completed task with a past due date is NOT overdue (matches Java behavior) |

### ANSI Color Precedence
When a task could match multiple visual categories, the precedence order is:
1. **Overdue** (yellow `\033[33m`) — checked first via `task.is_overdue()`
2. **DONE** (green `\033[32m`)
3. **CANCELLED** (dim `\033[2m`)
4. **Default** (no color / reset `\033[0m`) — TODO and IN_PROGRESS

This is logically clean because `is_overdue()` already excludes DONE and CANCELLED tasks, so in practice there's no ambiguity. But the code should still check overdue first as a defensive measure.

### No Input Validation Needed
- **List tasks** (option 2): no input, just displays
- **Show summary** (option 9): no input, just displays
- **Show overdue** (option 10): no input, just displays
- Menu input validation (invalid option → "Invalid option.") is already handled by the existing `main.cpp` dispatch logic

### Test Coverage for Edge Cases
The new tests in `task_service_test.cpp` should cover:
- `getTasksSortedByPriority()` returns correct order with mixed priorities
- `getTasksSortedByPriority()` returns empty vector when no tasks exist
- `getOverdueTasks()` returns only overdue tasks (excludes DONE/CANCELLED with past dates)
- `getOverdueTasks()` returns empty vector when no tasks are overdue
- `getSummary()` returns correct counts across all statuses
- `getSummary()` returns all-zero struct when store is empty
