# Validation

## Validation & Error Handling — Delete a Task

### Input Validation Flow
The `removeTask()` function in `main.cpp` will validate in this order:

1. **Parse task ID** — Use existing `readId()` helper. If input is non-numeric, print `Invalid ID.` and return (already implemented).
2. **Check task exists** — Call `service.getTask(id)`. If `std::nullopt`, print `Task not found.` and return **without** showing the confirmation prompt.
3. **Confirmation prompt** — Only shown for valid, existing tasks. Ask `Are you sure you want to delete task #N? (y/n): `
4. **Execute or cancel** — If `y`/`Y`, call `service.deleteTask(id)` and print `Task N removed.`. Otherwise print `Delete cancelled.`

### Edge Cases

| Scenario | Behavior |
|---|---|
| Non-numeric input (e.g., `abc`) | `Invalid ID.` — handled by existing `readId()` |
| Negative ID (e.g., `-1`) | Passes `readId()` parse, but `getTask(-1)` returns `nullopt` → `Task not found.` |
| ID = 0 | Same as above — `Task not found.` |
| Valid ID, user types `y` or `Y` | Task deleted, `Task N removed.` |
| Valid ID, user types `n`, `N`, or anything else | `Delete cancelled.` — no deletion |
| Valid ID, user presses Enter (empty input) | Treated as rejection → `Delete cancelled.` |
| Task deleted between check and confirm (impossible in single-threaded CLI) | N/A — not a concern |

### No Service-Layer Validation Changes
All validation for the delete operation lives in the CLI layer (`main.cpp`). The service and store layers already handle their contracts correctly:
- `TaskService::deleteTask(id)` returns `bool` (true if deleted, false if not found)
- `TaskStore::delete_task(id)` uses `std::unordered_map::erase()` which is safe for missing keys

### Alignment with Acceptance Criteria
| AC | Validation |
|---|---|
| AC1: Task permanently removed | Covered by `TaskStore::delete_task()` erasing from map + existing unit tests |
| AC2: Confirmation dialog in GUI → CLI Y/N prompt | New confirmation prompt before `deleteTask()` call |
| AC3: Non-existent ID fails with notification | Existence check before prompt + `Task not found.` message |
