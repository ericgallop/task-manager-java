# Locked Decisions for Story 056532ef-4367-4513-a22a-1c55af2e13c7

## Implementation Approach
## Implementation Approach — Create a New Task

### API Design: Default Parameters
`TaskService::createTask()` uses a **single method with default parameters** rather than Java's two overloads:

```cpp
std::optional<Task> createTask(
    const std::string& title,
    Priority priority = Priority::MEDIUM,
    const std::string& description = "",
    const std::string& due_date = "",
    const std::string& assignee = ""
);
```

- Returns `std::optional<Task>` — empty on validation failure (blank title, invalid date)
- Default priority is `MEDIUM`, matching Java behavior
- Empty strings represent "not provided" for description, due date, and assignee (no nulls in C++)

### Validation Centralized in Service Layer
All task-creation business rules live in `TaskService::createTask()`:
- **Title**: Reject blank/whitespace-only → return `std::nullopt`, print error to `std::cerr`
- **Priority**: Already resolved by caller or defaults to `MEDIUM` — no validation needed in `createTask` itself
- **Due date**: If non-empty, validated via a `date_utils` helper; invalid format → return `std::nullopt`, print error to `std::cerr`
- **Assignee/Description**: Stored as-is (no validation)

The CLI (`main.cpp`) is a thin input-gathering layer — it reads user input and passes it directly to the service. No business logic in the CLI.

### Priority Parsing: Free Function
Java's `Priority.fromString()` becomes a free function in `priority.h`:

```cpp
// In priority.h
enum class Priority { LOW, MEDIUM, HIGH, CRITICAL };
Priority priority_from_string(const std::string& s);  // returns MEDIUM for unrecognized input
std::string priority_to_string(Priority p);
```

This matches the Java behavior: unrecognized input silently defaults to `MEDIUM`.

### Date Handling: String-Based with Validation Utility
Per the locked technology stack decision, dates are `std::string` in `YYYY-MM-DD` format:
- A small `date_utils.h/.cpp` (or header-only) provides:
  - `bool is_valid_date(const std::string& s)` — checks format and basic validity (month 1-12, day 1-31, etc.)
  - `std::string today_as_string()` — returns current date as `YYYY-MM-DD` for the `created_date` field
- Lexicographic comparison works for ISO 8601 date strings (used later for overdue detection)

### Task Model (C++ struct/class)
The `Task` class holds all fields from Java, adapted for C++17 value semantics:

| Field | C++ Type | Default | Notes |
|---|---|---|---|
| `id` | `int` | Auto-assigned | From `TaskStore::next_id()` |
| `title` | `std::string` | *(required)* | Must be non-blank |
| `description` | `std::string` | `""` | Empty = not set |
| `priority` | `Priority` | `MEDIUM` | Enum class |
| `status` | `TaskStatus` | `TODO` | Enum class |
| `assignee` | `std::string` | `""` | Empty = unassigned |
| `due_date` | `std::string` | `""` | `YYYY-MM-DD` or empty |
| `created_date` | `std::string` | Today's date | Set at creation time |

### ID Generation
- `TaskStore` holds an `int next_id_` counter starting at 1 (replaces Java's `AtomicInteger` — no concurrency needed)
- `TaskService::createTask()` calls `store_.next_id()` to get the next ID, constructs the `Task`, then calls `store_.save(task)`

### Files Touched by This Story
| File | Purpose |
|---|---|
| `priority.h` | `enum class Priority` + `priority_from_string()` + `priority_to_string()` |
| `task_status.h` | `enum class TaskStatus` + `is_terminal()` helper |
| `task.h / task.cpp` | `Task` class with all fields, getters/setters, `to_string()` |
| `date_utils.h` | `is_valid_date()`, `today_as_string()` (header-only) |
| `task_store.h / task_store.cpp` | In-memory storage with `save()`, `find_by_id()`, `next_id()` |
| `task_service.h / task_service.cpp` | `createTask()` with validation logic |
| `main.cpp` | CLI `addTask()` flow — gathers input, calls service |
| `CMakeLists.txt` | Build configuration for all source files |
| `task_service_test.cpp` | Ported tests for task creation (from both Java test files) |

### CLI Flow for "Add Task" (menu option 1)
1. Prompt `"Title: "` → read line → pass to service (service validates non-blank)
2. Prompt `"Priority [LOW/MEDIUM/HIGH/CRITICAL] (default MEDIUM): "` → read line → `priority_from_string()`
3. Prompt `"Description (optional): "` → read line
4. Prompt `"Due date [yyyy-MM-dd] (optional): "` → read line
5. Prompt `"Assignee (optional): "` → read line
6. Call `service.createTask(title, priority, description, due_date, assignee)`
7. If result has value → print `"Created: "` + task; else error was already printed by service

## Validation
## Validation Rules — Create a New Task

All validation is enforced in `TaskService::createTask()`. The CLI gathers raw input and delegates to the service.

### Field-by-Field Validation

| Field | Rule | On Failure | AC Reference |
|---|---|---|---|
| **Title** | Must be non-empty after trimming whitespace | Return `std::nullopt`; print `"Title cannot be empty."` to `std::cerr` | AC #6 |
| **Priority** | Parsed via `priority_from_string()` before reaching service | Unrecognized value → defaults to `MEDIUM` (never fails) | AC #2 |
| **Description** | No validation — any string accepted | N/A (empty string = not set) | AC #3 |
| **Due date** | If non-empty, must match `YYYY-MM-DD` format and be a valid calendar date | Return `std::nullopt`; print `"Invalid date format — due date not set."` to `std::cerr` | AC #4 |
| **Assignee** | No validation — any non-empty string accepted | N/A (empty string = unassigned) | AC #5 |

### Date Validation Details (`is_valid_date`)
The `date_utils.h` utility function checks:
1. **Format**: Exactly 10 characters, pattern `DDDD-DD-DD` where D is a digit and `-` is literal
2. **Month range**: 01–12
3. **Day range**: 01–31 (basic check; no month-specific day count needed per AC — the AC only requires format validation of `yyyy-MM-dd`)
4. **No time component or timezone** — pure date string only

### Auto-Set Fields (No User Input)
| Field | Value | Rule |
|---|---|---|
| `id` | `TaskStore::next_id()` | Auto-incremented integer starting at 1 |
| `status` | `TaskStatus::TODO` | Always set on creation |
| `created_date` | `today_as_string()` | Current date in `YYYY-MM-DD` format |

### Priority Parsing Behavior
`priority_from_string()` in `priority.h`:
- Converts input to uppercase before matching
- Maps `"LOW"` → `LOW`, `"MEDIUM"` → `MEDIUM`, `"HIGH"` → `HIGH`, `"CRITICAL"` → `CRITICAL`
- **Empty string** → returns `MEDIUM` (default)
- **Any unrecognized value** → returns `MEDIUM` (silent default, matching Java behavior per AC #2)

### Error Reporting Pattern
- Validation errors are printed to `std::cerr` inside `TaskService::createTask()`
- The method returns `std::nullopt` so the CLI can detect failure without parsing error messages
- The CLI checks `result.has_value()` and only prints the success message if the task was created
- This keeps error messages co-located with the validation logic (single source of truth)

### Edge Cases
| Scenario | Behavior |
|---|---|
| Title is all whitespace (`"   "`) | Treated as blank → rejected |
| Priority is mixed case (`"high"`, `"High"`) | Uppercased → matches `HIGH` |
| Priority is gibberish (`"urgent"`) | Defaults to `MEDIUM` |
| Due date is empty string | No due date set (valid) |
| Due date is `"2024-13-01"` (month 13) | Rejected — invalid month |
| Due date is `"2024-1-5"` (no zero-padding) | Rejected — format must be `YYYY-MM-DD` (10 chars) |
| Due date is `"not-a-date"` | Rejected — doesn't match digit pattern |
| Assignee is empty | No assignee set (valid) |
