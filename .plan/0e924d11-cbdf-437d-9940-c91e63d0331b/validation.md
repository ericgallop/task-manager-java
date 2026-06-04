# Validation

## Validation & Edge Cases: Filter Tasks by Status

### 1. Invalid Filter Selection (CLI sub-menu)

When the user enters an invalid choice in the filter sub-menu (not 1–6):
- Display: `"Invalid filter option."`
- Return to the **main menu** (do not re-prompt the filter sub-menu)
- This matches the existing CLI pattern — invalid input on the main menu prints `"Invalid option."` and re-shows the menu

### 2. Empty Results

When a valid filter is selected but no tasks match:
- Display: `"No tasks match the selected filter."`
- This differentiates from the existing "List all tasks" empty message (`"No tasks."`) to make it clear a filter is active

### 3. Overdue Filter Logic

A task is considered **overdue** when ALL of these are true:
- `due_date` is non-empty (task has a due date set)
- `due_date < today` (the due date is in the past — lexicographic string comparison of `YYYY-MM-DD` format)
- `is_terminal(status)` is `false` (status is NOT `DONE` or `CANCELLED`)

This matches the existing `Task::is_overdue()` method already implemented in the C++ codebase. No new overdue logic is needed.

### 4. "All Tasks" Filter vs Menu Item 2

- **Menu item 2 ("List all tasks")**: Returns all tasks sorted by priority (existing behavior, unchanged)
- **Filter sub-menu option 1 ("All tasks")**: Returns all tasks — sorting behavior should match menu item 2 for consistency (sorted by priority descending)

### 5. Filter String Validation in `getFilteredTasks()`

The `getFilteredTasks(const std::string& filter)` method receives a controlled string from the CLI mapping (sub-menu choice 1–6 maps to a known string). However, for defensive coding:
- If an unrecognized filter string is passed, return an **empty vector**
- No exception or error — the CLI layer prevents invalid strings from reaching the service

### 6. No Input Validation Changes to Existing Code

This story does not modify any existing validation:
- Task creation validation (title cannot be blank) — unchanged
- Status transition validation (TransitionResult enum) — unchanged
- ID validation (readId helper) — unchanged
