# Implementation Approach

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
