# Locked Decisions for Story 771cd6eb-e4e0-4c1f-b036-8b46d03ad463

## Implementation Approach
## Implementation Approach — Delete a Task

### Current State
The delete functionality is **already implemented** across all layers of the C++ codebase:

| Layer | File | Method | Status |
|---|---|---|---|
| Storage | `task_store.cpp` | `delete_task(int id)` → erases from `std::unordered_map`, returns `bool` | ✅ Done |
| Service | `task_service.cpp` | `deleteTask(int id)` → delegates to `store_.delete_task(id)` | ✅ Done |
| CLI | `main.cpp` | `removeTask()` — menu option 8, prompts for ID, prints success/not-found | ⚠️ Needs update |
| Tests | `task_service_test.cpp` | Two tests: "removes task from store" and "returns false for non-existent ID" | ✅ Done |

### What Needs to Change
**Only one change** is required: modify the `removeTask()` function in `main.cpp` to add a **Y/N confirmation prompt** before executing the delete.

#### Current behavior (no confirmation):
```
Task ID to remove: 3
Task 3 removed.
```

#### New behavior (with confirmation):
```
Task ID to remove: 3
Are you sure you want to delete task #3? (y/n): y
Task 3 removed.
```

### Confirmation Prompt Details
- **Prompt text**: `Are you sure you want to delete task #3? (y/n): `
- **Accept**: `y` or `Y` (case-insensitive)
- **Reject**: Any other input (including `n`, `N`, empty) → prints `Delete cancelled.` and returns without deleting
- **Not-found check first**: If the task ID doesn't exist, print `Task not found.` immediately — don't show the confirmation prompt for a non-existent task. This requires calling `service.getTask(id)` before prompting.
- This mirrors the Java Swing GUI's `JOptionPane.showConfirmDialog()` pattern adapted for CLI

### No Changes to Service or Store Layers
The `TaskService::deleteTask()` and `TaskStore::delete_task()` methods are functionally complete and do not need modification. The confirmation is purely a UI-layer concern.

### No New Tests Needed
The existing two delete tests cover the service-layer behavior (successful delete and not-found). The confirmation prompt is CLI I/O that is not covered by unit tests (per the locked testing strategy: "No CLI output tests").

## Validation
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
