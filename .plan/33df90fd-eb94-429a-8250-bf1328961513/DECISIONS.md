# Locked Decisions for Story 33df90fd-eb94-429a-8250-bf1328961513

## Implementation Approach
## CLI Menu Loop — Direct Port of `Main.java::runCli()`

### What Gets Built
A single file `main.cpp` that implements the interactive CLI menu loop, directly porting the Java `Main.java` CLI behavior. This is the **entry point** of the entire C++ application.

### Scope Adjustment from Story
The locked **UI Framework** decision mandates CLI-only — no GUI. This changes the acceptance criteria:
- **AC1 (GUI on no args)** → **Dropped**. No GUI exists.
- **AC2 (`--cli` flag)** → **Modified**. The app launches directly into CLI mode with no flag needed. The `--cli` argument is ignored/unnecessary.
- **AC3 (10 menu operations)** → **Kept as-is**.
- **AC4 (exit with goodbye)** → **Kept as-is**.

### Implementation Pattern
Port the Java CLI structure 1:1 into C++:

| Java Pattern | C++ Equivalent |
|---|---|
| `Scanner scanner = new Scanner(System.in)` | `std::getline(std::cin, input)` |
| `scanner.nextLine().trim()` | `std::getline()` + a `trim()` utility function |
| `Integer.parseInt(...)` with `NumberFormatException` catch | `std::stoi()` with `std::invalid_argument` / `std::out_of_range` catch |
| `switch (input)` on String | `if/else if` chain on `std::string` (or `std::map` dispatch) |
| `System.out.println(...)` | `std::cout << ... << std::endl` |
| `Priority.fromString(s)` | Free function `priority_from_string(s)` returning `Priority::medium` on bad input |
| `Task.toString()` | `operator<<(std::ostream&, const Task&)` or `to_string()` method |

### CLI Menu Structure (matches Java exactly)
```
Task Manager CLI
================

 1. Add task
 2. List all tasks
 3. Start task
 4. Complete task
 5. Cancel task
 6. Assign task
 7. Set priority
 8. Remove task
 9. Summary
10. Show overdue
 0. Exit
Choose: 
```

### Key Implementation Details

1. **`main.cpp` owns the menu loop** — creates `TaskStore`, creates `TaskService`, then runs the loop. Each menu option is a free function (or a method on a helper class) that takes `TaskService&` and `std::cin`/`std::cout` references.

2. **Helper functions** mirror Java's static methods: `add_task()`, `list_tasks()`, `start_task()`, `complete_task()`, `cancel_task()`, `assign_task()`, `set_priority()`, `remove_task()`, `show_summary()`, `show_overdue()`, `read_id()`.

3. **String trimming utility** — a small `trim()` function (or inline lambda) since C++ has no built-in trim. Used on all `std::getline` input.

4. **No `--cli` flag** — `main()` ignores `argc`/`argv` entirely and goes straight into the menu loop.

5. **Graceful exit** — option "0" breaks the loop, prints "Goodbye!", and returns 0.

### Dependencies
This story depends on the model and service layers (`Task`, `TaskService`, `TaskStore`, `Priority`, `TaskStatus`, `TaskSummary`) being implemented first. The CLI is the topmost layer that wires everything together.

### Files Produced
- `main.cpp` — entry point + CLI menu loop + all menu handler functions

## Validation
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
