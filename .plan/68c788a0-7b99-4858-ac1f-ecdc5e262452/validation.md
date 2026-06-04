# Validation

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
