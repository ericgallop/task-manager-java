# Validation

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
