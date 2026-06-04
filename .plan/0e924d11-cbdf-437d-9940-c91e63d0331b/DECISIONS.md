# Locked Decisions for Story 0e924d11-cbdf-437d-9940-c91e63d0331b

## Implementation Approach
## Implementation Approach: Filter Tasks by Status

### Design Summary
Add a **single unified filtering method** to `TaskService` and a **new menu item "9. Filter tasks by status"** in the CLI that opens a sub-prompt listing the 6 filter options.

---

### 1. Service Layer — `TaskService`

**New method** added to `task_service.h / task_service.cpp`:

```cpp
std::vector<Task> getFilteredTasks(const std::string& filter) const;
```

- Accepts a string filter key: `"all"`, `"todo"`, `"in_progress"`, `"done"`, `"cancelled"`, `"overdue"`
- Returns a `std::vector<Task>` matching the filter
- Implementation delegates to existing `TaskStore` methods:
  - `"all"` → `store_.find_all()`
  - `"todo"` → `store_.find_by_status(TaskStatus::TODO)`
  - `"in_progress"` → `store_.find_by_status(TaskStatus::IN_PROGRESS)`
  - `"done"` → `store_.find_by_status(TaskStatus::DONE)`
  - `"cancelled"` → `store_.find_by_status(TaskStatus::CANCELLED)`
  - `"overdue"` → `store_.find_all()` filtered by `task.is_overdue()` (non-terminal tasks past their due date)

**Why a single method?** The `TaskStore` already has `find_by_status(TaskStatus)` for the four concrete statuses, and overdue is a computed filter across all tasks. A single entry point with a string selector is simpler than 6 separate methods, and maps cleanly to the CLI sub-menu choices.

### 2. CLI Layer — `main.cpp`

**New menu item** added to `printMenu()`:
```
 9. Filter tasks by status
```

When selected, a **sub-prompt** is displayed:
```
Filter by:
  1. All tasks
  2. Todo
  3. In progress
  4. Done
  5. Cancelled
  6. Overdue
Choose filter:
```

The user picks 1–6, and the filtered results are displayed using the same `task.to_string()` format as the existing "List all tasks" (menu item 2). If no tasks match the filter, display `"No tasks match the selected filter."`.

**Menu item 2 ("List all tasks") is unchanged** — it continues to show all tasks sorted by priority as before.

### 3. Files Modified

| File | Change |
|---|---|
| `task_service.h` | Add `getFilteredTasks(const std::string& filter)` declaration |
| `task_service.cpp` | Implement `getFilteredTasks` with switch/if-else on filter string |
| `main.cpp` | Add menu item 9, implement `filterTasks()` static function with sub-menu |

### 4. No Changes Required

- **`task_status.h`** — `TaskStatus` enum already has all 4 values (`TODO`, `IN_PROGRESS`, `DONE`, `CANCELLED`)
- **`task_store.h / task_store.cpp`** — `find_by_status(TaskStatus)` and `find_all()` already exist
- **`task.h / task.cpp`** — `is_overdue()` already exists
- **`priority.h`** — no changes

### 5. Test Additions

Per the locked testing strategy, tests are ported from Java. The Java source does not have dedicated filter-by-status tests (the existing tests cover `getPendingTasks`, `getOverdueTasks`, etc. individually). For this story:

- Add **one new test case** in `task_service_test.cpp` verifying `getFilteredTasks()` returns correct results for each of the 6 filter options
- Add **one new E2E test case** in `cli_e2e_test.cpp` verifying the CLI sub-menu flow (input "9" → select filter → verify output)
- These are minimal additions that validate the new unified method and CLI integration

## Validation
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
