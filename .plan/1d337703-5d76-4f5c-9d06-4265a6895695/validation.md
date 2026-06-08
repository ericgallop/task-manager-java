# Validation

## Validation & Error Handling for TaskStore

This story's scope is the storage layer only. TaskStore is a thin data container — validation is minimal and follows the Java original's behavior.

### TaskStore Error Handling

| Method | Error Scenario | Behavior | Rationale |
|---|---|---|---|
| `save(task)` | ID already exists | Overwrite silently | Matches Java `LinkedHashMap.put()` — used for both insert and implicit update |
| `save(task)` | Any task passed in | Always succeeds | No field validation at storage layer (Java original has none either) |
| `find_by_id(id)` | ID not found | Returns `std::nullopt` | Idiomatic C++17, matches Java's `Optional.empty()` |
| `remove(id)` | ID not found | Returns `false` | Matches Java's `store.remove(id) != null` |
| `next_id()` | Counter overflow | No guard | At ≤100 tasks (per NFR), `int` overflow is not a practical concern |
| `find_all()` / `find_by_*()` | Empty store | Returns empty `std::vector` | Standard behavior, no special handling |

### What TaskStore Does NOT Validate
- **Task field validity** (empty title, null priority, etc.) — this is the responsibility of `Task` itself (Java's `Task.setTitle()` throws `IllegalArgumentException` on blank titles; the C++ equivalent will be handled in the Task model story)
- **Status transition rules** — enforced in `Task.startProgress()`, `Task.complete()`, etc., not in the store
- **Duplicate titles or content** — not validated in Java, not validated in C++

### Edge Cases Addressed by Design
1. **Deleted ID reuse** — IDs never reuse because `next_id_` only increments (AC #3)
2. **Insertion order preservation** — `std::map<int, Task>` with sequential keys guarantees ordered iteration (AC #2)
3. **Session-scoped ephemeral data** — `TaskStore` is a stack-allocated object in `main()`; process exit destroys all data (AC #4)

### No Exceptions Thrown
TaskStore itself throws no exceptions. All error conditions are communicated via return types (`bool`, `std::optional`). This is consistent with the locked migration strategy's principle of "stronger error handling — replace silent boolean returns with more descriptive feedback where practical" while keeping the storage layer simple — richer error feedback will be added at the service/CLI layers in later stories.
