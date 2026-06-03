# Validation

## Validation & Error Handling — Update Task Properties

### Existing Validation (No Changes Needed)
The current code already handles all validation correctly for this story:

#### Task Not Found (AC4)
- `TaskStore::update(int id, ...)` returns `false` when the ID doesn't exist in the `tasks_` map
- `TaskService::assignTask()` and `TaskService::updatePriority()` propagate this `false` return
- CLI already prints `"Task not found."` when either returns `false`

#### Priority Input Parsing
- `priority_from_string()` in `priority.h` handles case-insensitive parsing of `LOW`, `MEDIUM`, `HIGH`, `CRITICAL`
- Unrecognized input defaults to `MEDIUM` (matches Java behavior)
- No additional validation needed — all 4 AC3 priority values are already supported

#### Assignee Input
- Any string (including empty) is accepted as an assignee value
- Empty string (`""`) represents "no assignee" (C++ equivalent of Java's `null` assignee)
- No character or length validation on assignee names (matches Java behavior)

### Edge Cases Considered

| Edge Case | Behavior | Notes |
|---|---|---|
| Assign to empty string | Clears assignee | AC2 — works correctly via `set_assignee("")` |
| Assign whitespace-only name (e.g. `"  "`) | Stores whitespace as assignee | Matches Java behavior — no trimming on assignee. This is acceptable; the Java source doesn't trim either |
| Set priority with invalid string | Defaults to `MEDIUM` | `priority_from_string` fallback behavior |
| Negative task ID from CLI | Caught by `readId()` returning `-1`, operation skipped | Existing CLI guard |
| Task ID = 0 | Valid lookup, returns not-found if no task with ID 0 | Auto-increment starts at 1, so 0 will always be not-found |

### No New Validation Rules
This story does not introduce any new validation logic. The existing service-layer `bool` returns and CLI error messages fully satisfy all four acceptance criteria.
