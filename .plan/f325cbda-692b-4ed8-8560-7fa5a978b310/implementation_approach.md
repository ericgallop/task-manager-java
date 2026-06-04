# Implementation Approach

## Sort Tasks — Implementation Approach

### CLI Interaction Flow
The existing "List all tasks" menu option (option 2) gains a **sort sub-menu**. When the user selects option 2, a sub-prompt appears:

```
Sort by:
  1. Priority (highest first)
  2. Due date (earliest first)
  3. Creation order
Choose:
```

- **Invalid input** defaults to priority sort, with a message: `"Invalid choice, sorting by priority."`
- The menu label stays as `"List all tasks"` (no change to the main menu).

### Service Layer — New and Modified Methods

Three sort methods on `TaskService`, mapped to the sub-menu choices:

| Sub-menu | C++ Method | Behavior |
|---|---|---|
| 1. Priority | `get_tasks_sorted_by_priority()` | **Existing** (ported from Java `getTasksSortedByPriority()`). Returns all tasks sorted CRITICAL → HIGH → MEDIUM → LOW. Uses `std::sort` with a lambda comparing `Priority` enum values in descending order. |
| 2. Due date | `get_tasks_sorted_by_due_date()` | **Modified from Java**. The Java version *excludes* tasks without due dates. The C++ version must include them, placing tasks **without due dates after** those with due dates (per AC #2). Tasks with due dates are sorted earliest-first by lexicographic `std::string` comparison (ISO 8601 `YYYY-MM-DD`). |
| 3. Creation order | `get_tasks_sorted_by_creation_order()` | **New method**. Returns all tasks sorted by ID ascending. Since IDs are auto-incremented, this reflects insertion order. Effectively equivalent to `get_all_tasks()` if the store uses an ordered container, but explicitly sorted to guarantee the contract. |

### Priority Enum Design
```cpp
enum class Priority { LOW = 0, MEDIUM = 1, HIGH = 2, CRITICAL = 3 };
```
Ascending numeric values so that `std::sort` descending (using `>`) gives CRITICAL-first ordering.

### Due Date Sort — Handling Missing Dates
The due-date sort uses a custom comparator:
- Tasks **with** due dates come first, sorted by date string ascending (lexicographic comparison of `YYYY-MM-DD` strings works correctly for chronological order).
- Tasks **without** due dates (empty string `""`) come after all dated tasks.
- Among tasks without due dates, order is by ID ascending (stable tiebreaker).

### CLI Output
Task display uses the same `to_string()` / `operator<<` format as the rest of the app — no change to how individual tasks are rendered. The only new output is the sort sub-prompt itself.

### No Changes to Storage Layer
`TaskStore` does not need new methods — all three sort operations retrieve `find_all()` and sort in the service layer using `std::sort` with lambdas.

### Testing
Per the locked testing strategy, only the existing 19 JUnit tests are ported. The Java tests already include `getTasksSortedByPriority_highestFirst` which validates priority sorting. The due-date and creation-order sorts **do not have existing Java tests**, so no new tests are added for them. The sort sub-menu CLI interaction is also not tested (no CLI tests per strategy).
