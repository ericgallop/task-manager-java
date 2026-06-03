# Validation

## Validation & Testing — Lifecycle State Transitions

### State Transition Validation Rules

All validation is enforced **inside the `Task` methods** (`start_progress()`, `complete()`, `cancel()`), with `TransitionResult` propagated through `TaskStore` and `TaskService` to the CLI.

#### Allowed Transitions Matrix

| Current State | `start_progress()` | `complete()` | `cancel()` |
|---|---|---|---|
| **TODO** | ✅ → IN_PROGRESS | ✅ → DONE | ✅ → CANCELLED |
| **IN_PROGRESS** | ❌ InvalidTransition | ✅ → DONE | ✅ → CANCELLED |
| **DONE** | ❌ InvalidTransition | ❌ InvalidTransition | ❌ InvalidTransition |
| **CANCELLED** | ❌ InvalidTransition | ❌ InvalidTransition | ❌ InvalidTransition |

#### Validation Logic per Method

**`Task::start_progress()`**
- Guard: `if (status_ != TaskStatus::TODO) return TransitionResult::InvalidTransition;`
- Rejects: IN_PROGRESS (AC #6), DONE (AC #4), CANCELLED (AC #5)

**`Task::complete()`**
- Guard: `if (is_terminal(status_)) return TransitionResult::InvalidTransition;`
- Allows: TODO → DONE, IN_PROGRESS → DONE (AC #2)
- Rejects: DONE (AC #4), CANCELLED (AC #5)

**`Task::cancel()`**
- Guard: `if (is_terminal(status_)) return TransitionResult::InvalidTransition;`
- Allows: TODO → CANCELLED, IN_PROGRESS → CANCELLED (AC #3)
- Rejects: DONE (AC #4), CANCELLED (AC #5)

#### Not-Found Handling (AC #7)
- `TaskStore::update_status()` returns `TransitionResult::NotFound` when the task ID doesn't exist in the map
- This is checked before the transition lambda is ever called

#### CLI Error Messages
Each `TransitionResult` maps to a specific user-facing message:

| Scenario | CLI Message |
|---|---|
| Start success | "Task {id} is now IN_PROGRESS." |
| Complete success | "Task {id} completed." |
| Cancel success | "Task {id} cancelled." |
| Start — invalid transition | "Cannot start task — task must be in TODO status." |
| Complete — invalid transition | "Cannot complete task — task is already completed or cancelled." |
| Cancel — invalid transition | "Cannot cancel task — task is already completed or cancelled." |
| Any — not found | "Task not found." |

### Tests — Ported + New

#### Ported from Java (3 tests already exist in Java, not yet in C++)
These Java tests cover lifecycle transitions and need to be added to `task_service_test.cpp`:

| Java Test | C++ Test Name | What It Tests |
|---|---|---|
| `TaskServiceTest.startTask_changesStatusToInProgress` | `startTask changes status to IN_PROGRESS` | AC #1: TODO → IN_PROGRESS |
| `TaskServiceTest.completeTask_fromInProgress` | `completeTask from IN_PROGRESS changes status to DONE` | AC #2: IN_PROGRESS → DONE |
| `TaskServiceTest.cancelTask_fromTodo` | `cancelTask from TODO changes status to CANCELLED` | AC #3: TODO → CANCELLED |
| `TaskServiceTest.startTask_failsForMissingId` | `startTask returns NotFound for missing ID` | AC #7: non-existent ID |

#### New Tests Required by Acceptance Criteria
These transitions are **not tested in the Java source** but are required by the story's ACs:

| C++ Test Name | AC | What It Tests |
|---|---|---|
| `completeTask from TODO changes status to DONE` | AC #2 | Direct TODO → DONE |
| `cancelTask from IN_PROGRESS changes status to CANCELLED` | AC #3 | IN_PROGRESS → CANCELLED |
| `startTask on DONE task returns InvalidTransition` | AC #4 | DONE is terminal |
| `completeTask on DONE task returns InvalidTransition` | AC #4 | DONE is terminal |
| `cancelTask on DONE task returns InvalidTransition` | AC #4 | DONE is terminal |
| `startTask on CANCELLED task returns InvalidTransition` | AC #5 | CANCELLED is terminal |
| `completeTask on CANCELLED task returns InvalidTransition` | AC #5 | CANCELLED is terminal |
| `cancelTask on CANCELLED task returns InvalidTransition` | AC #5 | CANCELLED is terminal |
| `startTask on IN_PROGRESS task returns InvalidTransition` | AC #6 | Must be TODO to start |
| `completeTask returns NotFound for missing ID` | AC #7 | Non-existent ID |
| `cancelTask returns NotFound for missing ID` | AC #7 | Non-existent ID |

#### Test Assertions Pattern
All lifecycle tests follow this pattern:
```cpp
TEST_CASE("completeTask on DONE task returns InvalidTransition", "[lifecycle]") {
    TaskStore store;
    TaskService service(store);
    auto task = service.createTask("Task");
    REQUIRE(task.has_value());
    
    // First complete it
    REQUIRE(service.completeTask(task->get_id()) == TransitionResult::Success);
    
    // Try to complete again — should be rejected
    CHECK(service.completeTask(task->get_id()) == TransitionResult::InvalidTransition);
}
```

#### Impact on Existing Tests
The `getSummary_countsCorrectly` test (when ported in a later story) calls `completeTask` on a TODO task directly — this remains valid since TODO → DONE is an allowed transition. No existing ported test breaks.

### Edge Cases Handled
- **Double-start**: IN_PROGRESS → start() = InvalidTransition (AC #6)
- **Double-complete**: DONE → complete() = InvalidTransition (AC #4)  
- **Double-cancel**: CANCELLED → cancel() = InvalidTransition (AC #5)
- **Cross-terminal**: DONE → cancel() and CANCELLED → complete() both return InvalidTransition
- **Non-existent ID**: All three operations return NotFound (AC #7)
- **Invalid CLI input**: Non-numeric ID input handled by existing `readId()` helper (already implemented)
