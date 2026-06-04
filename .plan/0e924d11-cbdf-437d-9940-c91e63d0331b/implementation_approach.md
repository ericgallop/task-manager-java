# Implementation Approach

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
