# Validation

## Validation & Edge Cases for Task Summary

### Business Rules (from Acceptance Criteria)

**Overdue definition (AC #2 and #3):**
A task is overdue if and only if ALL three conditions are true:
1. `due_date` is set (non-empty string)
2. `due_date` is strictly before today (`due_date < today_date()`)
3. Status is NOT terminal — i.e., status is `TODO` or `IN_PROGRESS`

Tasks with status `DONE` or `CANCELLED` are **never** counted as overdue, regardless of due date.

### Edge Cases Handled

| Scenario | Expected Behavior |
|---|---|
| No tasks exist | All counts return 0 (zero-initialized struct) |
| Task has no due date (`due_date` empty) | Not counted as overdue |
| Task due today | **Not overdue** — overdue requires strictly past (`<`, not `<=`), matching Java's `isAfter()` semantics |
| Task due yesterday, status TODO | Counted as overdue |
| Task due yesterday, status DONE | NOT overdue (terminal status) |
| Task due yesterday, status CANCELLED | NOT overdue (terminal status) |
| Task due yesterday, status IN_PROGRESS | Counted as overdue |
| All tasks in same status | That status count equals `total`, others are 0 |
| Task added then immediately deleted | Not reflected in summary (deleted from store) |

### Input Validation
- `getSummary()` takes no input parameters — no input validation needed
- The method is purely a read operation over the current store contents
- No error states possible: an empty store simply returns all-zero `TaskSummary`

### Date String Validation
- `today_date()` always produces a valid `YYYY-MM-DD` string (system clock formatted via `strftime`)
- Task due dates are validated at input time (when creating/updating tasks, not in this story's scope)
- Lexicographic comparison of `YYYY-MM-DD` strings is correct for chronological ordering

### Consistency with Java Behavior
- Java uses `LocalDate.now().isAfter(dueDate)` — this is a **strict** comparison (today == due date → NOT overdue)
- C++ equivalent: `due_date_ < today_date()` — lexicographic `<` preserves the same strict semantics
- The `getSummary_countsCorrectly` test from Java will be ported, validating status counts (the test doesn't cover overdue specifically, but `getOverdueTasks_returnsOnlyOverdue` validates the `isOverdue()` logic used by summary)
