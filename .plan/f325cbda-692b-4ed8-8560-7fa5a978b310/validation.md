# Validation

## Sort Tasks — Validation & Edge Cases

### Sort Sub-Menu Input Validation
| Input | Behavior |
|---|---|
| `"1"`, `"2"`, `"3"` | Valid — execute the corresponding sort |
| Empty string (user just presses Enter) | Invalid — default to priority sort with message `"Invalid choice, sorting by priority."` |
| Any other string (e.g. `"4"`, `"abc"`, `"-1"`) | Invalid — default to priority sort with same message |

No re-prompting; invalid input immediately falls through to priority-sorted listing.

### Edge Cases by Sort Type

**Priority sort:**
- **All same priority**: Tasks with the same priority appear in unspecified order (no secondary sort required by the ACs).
- **Empty task list**: Prints `"No tasks."` and returns to main menu (existing behavior, unchanged).

**Due date sort:**
- **No tasks have due dates**: All tasks are "without due dates" — they appear sorted by ID ascending.
- **All tasks have due dates**: Standard earliest-first ordering.
- **Mixed**: Dated tasks first (earliest-first), then undated tasks (by ID ascending).
- **Same due date**: Tasks sharing a due date appear in unspecified order among themselves.
- **Empty task list**: Prints `"No tasks."` (same as above).

**Creation order sort:**
- **Empty task list**: Prints `"No tasks."` (same as above).
- Straightforward ascending ID sort — no edge cases beyond empty list.

### No New Validation Elsewhere
- No changes to task creation, status transitions, or any other existing validation logic.
- The sort feature is read-only — it does not modify any task data, so no data integrity concerns.
