# Validation

## Validation & Error Handling — Match Java Behavior Exactly

All input validation and error messages are direct ports of the Java CLI. No additional guards or improved messages.

### Menu Input
- Valid inputs: `"0"` through `"10"` (string comparison after trimming)
- Any other input: print `"Invalid option."` and re-display menu

### Task ID Input (`readId` equivalent)
- Read line, trim, attempt `std::stoi()`
- On failure (non-numeric, empty): print `"Invalid ID."` and return `-1` to abort the operation
- No range checking — negative IDs and zero are accepted and passed to `TaskService`, which returns `false` (task not found)

### Add Task Validation
- **Title**: if empty after trim → `"Title cannot be empty."`, abort
- **Priority**: passed to `priority_from_string()` — unrecognized input defaults to `MEDIUM` (matches Java `Priority.fromString()`)
- **Description**: empty string treated as no description (`std::nullopt` or empty string passed to service)
- **Due date**: if non-empty, attempt parse as `YYYY-MM-DD`; on failure → `"Invalid date format — skipping due date."`, task created without due date

### Operation Feedback Messages (exact strings)
| Operation | Success | Failure |
|---|---|---|
| Start task | `"Task {id} is now IN_PROGRESS."` | `"Task not found or already started."` |
| Complete task | `"Task {id} completed."` | `"Task not found."` |
| Cancel task | `"Task {id} cancelled."` | `"Task not found."` |
| Assign task | `"Assigned task {id} to {name}"` | `"Task not found."` |
| Set priority | `"Priority updated."` | `"Task not found."` |
| Remove task | `"Task {id} removed."` | `"Task not found."` |
| List tasks (empty) | `"No tasks."` | — |
| Show overdue (empty) | `"No overdue tasks."` | — |

### Edge Cases Preserved from Java
- `startTask` on a non-TODO task: Java's `TaskService.startTask()` calls `startProgress()` which throws `IllegalStateException` — the C++ port should handle this equivalently (return `false` or catch and report)
- Empty assignee string: passed through to service as-is (matches Java behavior)
- `std::cin` EOF / stream failure: not explicitly handled in Java (`Scanner` would throw) — in C++ the menu loop should exit gracefully if `std::getline` fails (EOF), similar to reaching end of input
