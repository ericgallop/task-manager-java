# Locked Decisions for Story 59b60f9a-21a8-4f6e-bdb0-16377cd548e6

## Implementation Approach
## Implementation Approach — Update Task Properties

### Current State
The core service, store, and model code for assigning tasks and updating priority **already exists** in the C++ codebase:
- `TaskService::assignTask(int id, const std::string& assignee)` → calls `TaskStore::update()` which calls `Task::set_assignee()`
- `TaskService::updatePriority(int id, Priority priority)` → calls `TaskStore::update()` which calls `Task::set_priority()`
- Both return `bool` — `true` if the task was found, `false` otherwise
- `TaskStore::update()` uses a generic lambda-based mutator pattern

### What Needs to Change

#### 1. CLI Feedback for Clearing Assignee (`main.cpp`)
The `assignTask()` function in `main.cpp` needs to detect when the user enters an empty string and provide appropriate feedback:
- **Empty input** → print `"Cleared assignee from task X"` instead of `"Assigned task X to "`
- **Non-empty input** → keep existing `"Assigned task X to <name>"` message

This is the **only production code change** required.

#### 2. Tests (`task_service_test.cpp`)
Port the 2 Java tests and add tests for acceptance criteria not covered by Java:

| AC | Test | Source |
|---|---|---|
| AC1: Assign by name | `assignTask sets assignee` | Ported from `TaskServiceTest.assignTask_setsAssignee` |
| AC2: Clear assignee | `assignTask with empty string clears assignee` | **New** — no Java equivalent |
| AC3: Change priority | `updatePriority changes priority` | Ported from `TaskServiceTest.updatePriority_changesPriority` |
| AC4: Not found (assign) | `assignTask returns false for missing ID` | **New** — no Java equivalent |
| AC4: Not found (priority) | `updatePriority returns false for missing ID` | **New** — no Java equivalent |

Total: **5 tests** added to `task_service_test.cpp` (2 ported from Java, 3 new for uncovered ACs).

#### 3. No Changes Needed
- **`task_service.h` / `task_service.cpp`** — no changes; `assignTask` and `updatePriority` already work correctly
- **`task_store.h` / `task_store.cpp`** — no changes; `update()` handles the mutation
- **`task.h` / `task.cpp`** — no changes; setters already exist
- **`priority.h`** — no changes; all 4 priority levels already defined
- **`CMakeLists.txt`** — no changes needed

### Implementation Order
1. Update `assignTask()` in `main.cpp` — add empty-input detection and differentiated feedback message
2. Add 5 test cases to `task_service_test.cpp`
3. Build and run tests to verify

## Validation
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
