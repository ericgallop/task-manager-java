#include "doctest/doctest.h"

#include "task_store.h"

// ============================================================================
// AC1 — Unique monotonically increasing IDs
// ============================================================================

TEST_CASE("next_id returns 1 on first call") {
    TaskStore store;
    CHECK(store.next_id() == 1);
}

TEST_CASE("next_id returns monotonically increasing integers") {
    TaskStore store;
    int id1 = store.next_id();
    int id2 = store.next_id();
    int id3 = store.next_id();
    CHECK(id1 == 1);
    CHECK(id2 == 2);
    CHECK(id3 == 3);
}

TEST_CASE("next_id increments independently of save calls") {
    TaskStore store;
    int id1 = store.next_id();  // 1
    store.next_id();            // 2 — intentionally unused
    // Save only the first, skip the second
    store.save(Task(id1, "First"));
    int id3 = store.next_id();  // 3
    CHECK(id3 == 3);
}

// ============================================================================
// AC2 — Insertion order preserved
// ============================================================================

TEST_CASE("find_all returns tasks in insertion order") {
    TaskStore store;
    int id1 = store.next_id();
    int id2 = store.next_id();
    int id3 = store.next_id();

    store.save(Task(id1, "First"));
    store.save(Task(id2, "Second"));
    store.save(Task(id3, "Third"));

    auto all = store.find_all();
    REQUIRE(all.size() == 3);
    CHECK(all[0].id() == id1);
    CHECK(all[0].title() == "First");
    CHECK(all[1].id() == id2);
    CHECK(all[1].title() == "Second");
    CHECK(all[2].id() == id3);
    CHECK(all[2].title() == "Third");
}

TEST_CASE("find_all preserves order even with gaps from deletions") {
    TaskStore store;
    int id1 = store.next_id();
    int id2 = store.next_id();
    int id3 = store.next_id();

    store.save(Task(id1, "First"));
    store.save(Task(id2, "Second"));
    store.save(Task(id3, "Third"));
    store.remove(id2);

    auto all = store.find_all();
    REQUIRE(all.size() == 2);
    CHECK(all[0].id() == id1);
    CHECK(all[1].id() == id3);
}

// ============================================================================
// AC3 — IDs never reused after deletion
// ============================================================================

TEST_CASE("IDs are never reused after deletion") {
    TaskStore store;
    int id1 = store.next_id();  // 1
    int id2 = store.next_id();  // 2
    int id3 = store.next_id();  // 3

    store.save(Task(id1, "Task 1"));
    store.save(Task(id2, "Task 2"));
    store.save(Task(id3, "Task 3"));

    // Delete task 2
    store.remove(id2);

    // Next ID should be 4, not 2
    int id4 = store.next_id();
    CHECK(id4 == 4);

    // Save with the new ID
    store.save(Task(id4, "Task 4"));

    // Verify deleted ID is gone
    auto found = store.find_by_id(id2);
    CHECK_FALSE(found.has_value());

    // Verify new task exists
    auto found4 = store.find_by_id(id4);
    REQUIRE(found4.has_value());
    CHECK(found4->get().title() == "Task 4");

    // Verify total count
    auto all = store.find_all();
    CHECK(all.size() == 3);
}

TEST_CASE("deleting all tasks does not reset ID counter") {
    TaskStore store;
    int id1 = store.next_id();
    int id2 = store.next_id();

    store.save(Task(id1, "A"));
    store.save(Task(id2, "B"));
    store.remove(id1);
    store.remove(id2);

    CHECK(store.find_all().empty());

    // Counter should continue from 3, not restart at 1
    int id3 = store.next_id();
    CHECK(id3 == 3);
}

// ============================================================================
// AC4 — Ephemeral storage (session-scoped)
// ============================================================================

TEST_CASE("freshly constructed TaskStore is empty") {
    TaskStore store;
    CHECK(store.find_all().empty());
}

TEST_CASE("freshly constructed TaskStore starts next_id at 1") {
    TaskStore store;
    CHECK(store.next_id() == 1);
}

TEST_CASE("two independent TaskStore instances have separate state") {
    TaskStore store1;
    TaskStore store2;

    store1.save(Task(store1.next_id(), "Store1 Task"));

    CHECK(store1.find_all().size() == 1);
    CHECK(store2.find_all().empty());
    CHECK(store2.next_id() == 1);  // store2's counter is independent
}

// ============================================================================
// save() — method-level tests
// ============================================================================

TEST_CASE("save stores task and returns reference to stored copy") {
    TaskStore store;
    Task original(1, "My Task");
    Task& stored = store.save(original);

    CHECK(stored.id() == 1);
    CHECK(stored.title() == "My Task");
}

TEST_CASE("save overwrites existing task with same ID silently") {
    TaskStore store;
    store.save(Task(1, "Original"));
    store.save(Task(1, "Updated"));

    auto all = store.find_all();
    REQUIRE(all.size() == 1);
    CHECK(all[0].title() == "Updated");
}

TEST_CASE("save stores a copy — modifying the original does not affect store") {
    TaskStore store;
    Task original(1, "Before");
    store.save(original);

    original.set_title("After");

    auto found = store.find_by_id(1);
    REQUIRE(found.has_value());
    CHECK(found->get().title() == "Before");
}

// ============================================================================
// find_by_id() — method-level tests
// ============================================================================

TEST_CASE("find_by_id returns nullopt for non-existent ID") {
    TaskStore store;
    CHECK_FALSE(store.find_by_id(999).has_value());
}

TEST_CASE("find_by_id returns nullopt for empty store") {
    TaskStore store;
    CHECK_FALSE(store.find_by_id(1).has_value());
}

TEST_CASE("find_by_id returns the correct task") {
    TaskStore store;
    store.save(Task(1, "First"));
    store.save(Task(2, "Second"));

    auto found = store.find_by_id(2);
    REQUIRE(found.has_value());
    CHECK(found->get().id() == 2);
    CHECK(found->get().title() == "Second");
}

TEST_CASE("find_by_id returns mutable reference — in-place mutation works") {
    TaskStore store;
    store.save(Task(1, "Original Title"));

    // Mutate via the returned reference
    auto found = store.find_by_id(1);
    REQUIRE(found.has_value());
    found->get().set_title("Modified Title");

    // Verify the change persists in the store
    auto found_again = store.find_by_id(1);
    REQUIRE(found_again.has_value());
    CHECK(found_again->get().title() == "Modified Title");

    // Also verify via find_all
    auto all = store.find_all();
    REQUIRE(all.size() == 1);
    CHECK(all[0].title() == "Modified Title");
}

TEST_CASE("find_by_id returns nullopt after task is removed") {
    TaskStore store;
    store.save(Task(1, "To Delete"));
    store.remove(1);

    CHECK_FALSE(store.find_by_id(1).has_value());
}

// ============================================================================
// find_all() — method-level tests
// ============================================================================

TEST_CASE("find_all returns empty vector for empty store") {
    TaskStore store;
    auto all = store.find_all();
    CHECK(all.empty());
}

TEST_CASE("find_all returns copies — modifying returned vector does not affect store") {
    TaskStore store;
    store.save(Task(1, "Task 1"));
    store.save(Task(2, "Task 2"));

    auto all = store.find_all();
    REQUIRE(all.size() == 2);

    // Modify the returned copies
    all[0].set_title("Changed");
    all.clear();

    // Store should be unaffected
    auto all_again = store.find_all();
    REQUIRE(all_again.size() == 2);
    CHECK(all_again[0].title() == "Task 1");
}

// ============================================================================
// find_by_status() — method-level tests
// ============================================================================

TEST_CASE("find_by_status returns only matching tasks") {
    TaskStore store;

    Task t1(1, "Todo task");
    store.save(t1);

    Task t2(2, "In progress task");
    t2.set_status(TaskStatus::IN_PROGRESS);
    store.save(t2);

    Task t3(3, "Done task");
    t3.set_status(TaskStatus::DONE);
    store.save(t3);

    Task t4(4, "Another todo");
    store.save(t4);

    auto todos = store.find_by_status(TaskStatus::TODO);
    REQUIRE(todos.size() == 2);
    CHECK(todos[0].id() == 1);
    CHECK(todos[1].id() == 4);

    auto in_progress = store.find_by_status(TaskStatus::IN_PROGRESS);
    REQUIRE(in_progress.size() == 1);
    CHECK(in_progress[0].id() == 2);

    auto done = store.find_by_status(TaskStatus::DONE);
    REQUIRE(done.size() == 1);
    CHECK(done[0].id() == 3);
}

TEST_CASE("find_by_status returns empty vector when no tasks match") {
    TaskStore store;
    store.save(Task(1, "A TODO task"));  // defaults to TODO

    auto cancelled = store.find_by_status(TaskStatus::CANCELLED);
    CHECK(cancelled.empty());
}

TEST_CASE("find_by_status returns empty vector for empty store") {
    TaskStore store;
    CHECK(store.find_by_status(TaskStatus::TODO).empty());
}

// ============================================================================
// find_by_priority() — method-level tests
// ============================================================================

TEST_CASE("find_by_priority returns only matching tasks") {
    TaskStore store;

    Task t1(1, "Low priority");
    t1.set_priority(Priority::LOW);
    store.save(t1);

    Task t2(2, "High priority");
    t2.set_priority(Priority::HIGH);
    store.save(t2);

    Task t3(3, "Medium priority");
    // default is MEDIUM
    store.save(t3);

    Task t4(4, "Critical priority");
    t4.set_priority(Priority::CRITICAL);
    store.save(t4);

    auto low = store.find_by_priority(Priority::LOW);
    REQUIRE(low.size() == 1);
    CHECK(low[0].id() == 1);

    auto high = store.find_by_priority(Priority::HIGH);
    REQUIRE(high.size() == 1);
    CHECK(high[0].id() == 2);

    auto medium = store.find_by_priority(Priority::MEDIUM);
    REQUIRE(medium.size() == 1);
    CHECK(medium[0].id() == 3);

    auto critical = store.find_by_priority(Priority::CRITICAL);
    REQUIRE(critical.size() == 1);
    CHECK(critical[0].id() == 4);
}

TEST_CASE("find_by_priority returns empty vector when no tasks match") {
    TaskStore store;
    store.save(Task(1, "Default medium"));  // defaults to MEDIUM

    CHECK(store.find_by_priority(Priority::CRITICAL).empty());
}

TEST_CASE("find_by_priority returns empty vector for empty store") {
    TaskStore store;
    CHECK(store.find_by_priority(Priority::MEDIUM).empty());
}

// ============================================================================
// find_by_assignee() — method-level tests
// ============================================================================

TEST_CASE("find_by_assignee returns only matching tasks") {
    TaskStore store;

    Task t1(1, "Alice's task");
    t1.set_assignee("alice");
    store.save(t1);

    Task t2(2, "Bob's task");
    t2.set_assignee("bob");
    store.save(t2);

    Task t3(3, "Another for Alice");
    t3.set_assignee("alice");
    store.save(t3);

    auto alice_tasks = store.find_by_assignee("alice");
    REQUIRE(alice_tasks.size() == 2);
    CHECK(alice_tasks[0].id() == 1);
    CHECK(alice_tasks[1].id() == 3);

    auto bob_tasks = store.find_by_assignee("bob");
    REQUIRE(bob_tasks.size() == 1);
    CHECK(bob_tasks[0].id() == 2);
}

TEST_CASE("find_by_assignee returns empty vector when no tasks match") {
    TaskStore store;
    Task t1(1, "Assigned to alice");
    t1.set_assignee("alice");
    store.save(t1);

    CHECK(store.find_by_assignee("charlie").empty());
}

TEST_CASE("find_by_assignee matches unassigned tasks with empty string") {
    TaskStore store;
    store.save(Task(1, "Unassigned"));  // assignee defaults to ""

    auto unassigned = store.find_by_assignee("");
    REQUIRE(unassigned.size() == 1);
    CHECK(unassigned[0].id() == 1);
}

TEST_CASE("find_by_assignee returns empty vector for empty store") {
    TaskStore store;
    CHECK(store.find_by_assignee("anyone").empty());
}

// ============================================================================
// remove() — method-level tests
// ============================================================================

TEST_CASE("remove returns true for existing ID") {
    TaskStore store;
    store.save(Task(1, "Removable"));
    CHECK(store.remove(1) == true);
}

TEST_CASE("remove returns false for non-existent ID") {
    TaskStore store;
    CHECK(store.remove(999) == false);
}

TEST_CASE("remove returns false for empty store") {
    TaskStore store;
    CHECK(store.remove(1) == false);
}

TEST_CASE("remove actually removes the task from the store") {
    TaskStore store;
    store.save(Task(1, "A"));
    store.save(Task(2, "B"));
    store.save(Task(3, "C"));

    store.remove(2);

    auto all = store.find_all();
    REQUIRE(all.size() == 2);
    CHECK(all[0].id() == 1);
    CHECK(all[1].id() == 3);

    CHECK_FALSE(store.find_by_id(2).has_value());
}

TEST_CASE("remove same ID twice — second call returns false") {
    TaskStore store;
    store.save(Task(1, "Once"));
    CHECK(store.remove(1) == true);
    CHECK(store.remove(1) == false);
}

// ============================================================================
// Integration: full workflow test
// ============================================================================

TEST_CASE("full workflow: create, query, filter, mutate, delete") {
    TaskStore store;

    // Create tasks using next_id
    int id1 = store.next_id();
    int id2 = store.next_id();
    int id3 = store.next_id();

    Task t1(id1, "Design API");
    t1.set_priority(Priority::HIGH);
    t1.set_assignee("alice");
    store.save(t1);

    Task t2(id2, "Write tests");
    t2.set_priority(Priority::MEDIUM);
    t2.set_assignee("bob");
    store.save(t2);

    Task t3(id3, "Deploy");
    t3.set_priority(Priority::HIGH);
    t3.set_assignee("alice");
    t3.set_status(TaskStatus::DONE);
    store.save(t3);

    // Verify find_all
    auto all = store.find_all();
    CHECK(all.size() == 3);

    // Filter by status
    auto todo = store.find_by_status(TaskStatus::TODO);
    CHECK(todo.size() == 2);

    auto done = store.find_by_status(TaskStatus::DONE);
    REQUIRE(done.size() == 1);
    CHECK(done[0].title() == "Deploy");

    // Filter by priority
    auto high = store.find_by_priority(Priority::HIGH);
    CHECK(high.size() == 2);

    // Filter by assignee
    auto alice = store.find_by_assignee("alice");
    CHECK(alice.size() == 2);

    // Mutate via find_by_id reference
    auto found = store.find_by_id(id1);
    REQUIRE(found.has_value());
    found->get().set_status(TaskStatus::IN_PROGRESS);

    auto in_progress = store.find_by_status(TaskStatus::IN_PROGRESS);
    REQUIRE(in_progress.size() == 1);
    CHECK(in_progress[0].id() == id1);

    // Delete and verify
    CHECK(store.remove(id2) == true);
    CHECK(store.find_all().size() == 2);
    CHECK_FALSE(store.find_by_id(id2).has_value());

    // Next ID continues past deleted
    int id4 = store.next_id();
    CHECK(id4 == 4);
}
