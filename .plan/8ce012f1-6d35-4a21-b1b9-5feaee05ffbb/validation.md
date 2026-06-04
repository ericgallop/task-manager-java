# Validation

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
