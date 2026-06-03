# Implementation Approach

## Implementation Approach — Task Lifecycle State Transitions

### Overview
Refactor the existing `Task`, `TaskService`, `TaskStore`, and CLI (`main.cpp`) to enforce the full task lifecycle state machine with descriptive feedback via a `TransitionResult` enum. The existing code already has `startTask/completeTask/cancelTask` methods, but they lack proper state validation and error reporting.

### Key Design Decision: `TransitionResult` Enum
A new `enum class TransitionResult` is added to `task_status.h` (alongside `TaskStatus`):

```cpp
enum class TransitionResult {
    Success,          // Transition applied
    NotFound,         // Task ID does not exist
    InvalidTransition // Transition not allowed from current state
};
```

This replaces the current `bool` return values and provides structured, specific feedback to the CLI layer.

### State Machine Rules
The valid transitions enforce the full lifecycle:

```mermaid
stateDiagram-v2
    [*] --> TODO
    TODO --> IN_PROGRESS: start()
    TODO --> DONE: complete()
    TODO --> CANCELLED: cancel()
    IN_PROGRESS --> DONE: complete()
    IN_PROGRESS --> CANCELLED: cancel()
    DONE --> [*]
    CANCELLED --> [*]
```

Rejected transitions:
- **DONE → any**: "Completed tasks cannot be modified" (AC #4)
- **CANCELLED → any**: "Cancelled tasks cannot be modified" (AC #5)
- **IN_PROGRESS → IN_PROGRESS** (start): "Task must be in TODO to start" (AC #6)

### Layer-by-Layer Changes

#### 1. `task_status.h` — Add `TransitionResult`
- Add `enum class TransitionResult { Success, NotFound, InvalidTransition }` to the existing header
- Optionally add `inline std::string transition_result_to_string(TransitionResult)` for debug/logging

#### 2. `task.h / task.cpp` — Full State Machine in Task Methods
Refactor the three transition methods to return `TransitionResult` instead of `void`:

| Method | Current Behavior | New Behavior |
|---|---|---|
| `start_progress()` | Throws `std::logic_error` if not TODO | Returns `InvalidTransition` if not TODO; returns `Success` and sets IN_PROGRESS otherwise |
| `complete()` | Unconditionally sets DONE (no validation!) | Returns `InvalidTransition` if DONE or CANCELLED; returns `Success` and sets DONE otherwise |
| `cancel()` | Unconditionally sets CANCELLED (no validation!) | Returns `InvalidTransition` if DONE or CANCELLED; returns `Success` and sets CANCELLED otherwise |

No more exceptions thrown from these methods — pure return values.

#### 3. `task_store.h / task_store.cpp` — Propagate `TransitionResult`
The `update()` method currently takes a `std::function<void(Task&)>` mutator and returns `bool`.

Change: Create a new overload or modify the mutator signature for transition operations:
```cpp
TransitionResult update_status(int id, std::function<TransitionResult(Task&)> transition);
```
- Returns `TransitionResult::NotFound` if the ID doesn't exist
- Otherwise calls the transition lambda and returns its result
- The existing `update()` with `void` mutator remains for non-transition operations (assignTask, updatePriority)

#### 4. `task_service.h / task_service.cpp` — Return `TransitionResult`
Change signatures:
```cpp
// Before (current)
bool startTask(int id);
bool completeTask(int id);
bool cancelTask(int id);

// After
TransitionResult startTask(int id);
TransitionResult completeTask(int id);
TransitionResult cancelTask(int id);
```

Each method calls `store_.update_status(id, [](Task& t) { return t.start_progress(); })` etc.

#### 5. `main.cpp` — Specific Error Messages per AC
The CLI handlers switch on the `TransitionResult` to provide AC-specific messages:

```cpp
static void startTask(TaskService& service) {
    int id = readId("Task ID to start: ");
    if (id < 0) return;
    auto result = service.startTask(id);
    switch (result) {
        case TransitionResult::Success:
            std::cout << "Task " << id << " is now IN_PROGRESS." << std::endl;
            break;
        case TransitionResult::NotFound:
            std::cout << "Task not found." << std::endl;
            break;
        case TransitionResult::InvalidTransition:
            std::cout << "Cannot start task — task must be in TODO status." << std::endl;
            break;
    }
}
```

Similar patterns for `completeTask` and `cancelTask`, with messages tailored to each AC:
- Complete on DONE/CANCELLED: "Cannot complete task — completed/cancelled tasks cannot be modified."
- Cancel on DONE/CANCELLED: "Cannot cancel task — completed/cancelled tasks cannot be modified."
- Start on IN_PROGRESS: "Cannot start task — task must be in TODO status."

### Files Modified (no new files)
| File | Change |
|---|---|
| `src/task_status.h` | Add `TransitionResult` enum |
| `src/task.h` | Change return types of `start_progress()`, `complete()`, `cancel()` from `void` to `TransitionResult` |
| `src/task.cpp` | Implement state validation in all three methods, remove exception throws |
| `src/task_store.h` | Add `update_status()` method returning `TransitionResult` |
| `src/task_store.cpp` | Implement `update_status()` |
| `src/task_service.h` | Change `startTask/completeTask/cancelTask` return type from `bool` to `TransitionResult` |
| `src/task_service.cpp` | Use `update_status()` and return `TransitionResult` |
| `src/main.cpp` | Update CLI handlers to switch on `TransitionResult` with specific messages |
| `tests/task_service_test.cpp` | Add lifecycle transition tests (see Validation decision) |
