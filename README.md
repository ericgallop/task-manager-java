# Task Manager Java

A desktop task management application built in Java Swing, with the original command-line interface still available. Supports creating, assigning, prioritizing, and tracking tasks through a defined lifecycle.

## Features

- Create tasks with title, description, priority, and due date
- Track tasks through statuses: `TODO` → `IN_PROGRESS` → `DONE` / `CANCELLED`
- Assign tasks to team members
- Filter and sort by priority or due date
- Identify overdue tasks
- Summary view with counts by status
- Desktop UI with task table, filters, sorting, and quick actions

## Project Structure

```
src/
├── main/java/com/demo/
│   ├── Main.java                   # App entry point (UI by default, CLI with --cli)
│   ├── TaskManagerUI.java          # Swing desktop interface
│   ├── Task.java                   # Task model
│   ├── TaskStatus.java             # Status enum (TODO, IN_PROGRESS, DONE, CANCELLED)
│   ├── Priority.java               # Priority enum (LOW, MEDIUM, HIGH, CRITICAL)
│   ├── TaskRepository.java         # Repository interface
│   ├── InMemoryTaskRepository.java # In-memory implementation
│   ├── TaskService.java            # Business logic
│   ├── TaskManager.java            # Facade (backward compatibility)
│   └── TaskSummary.java            # Summary value object
└── test/java/com/demo/
    ├── TaskManagerTest.java
    └── TaskServiceTest.java
```

## Requirements

- Java 11+
- Maven 3.6+

## Build & Run

```bash
# Build
mvn package -q

# Run the desktop UI
java -cp target/task-manager-1.0.0.jar com.demo.Main

# Run the CLI
java -cp target/task-manager-1.0.0.jar com.demo.Main --cli

# Run tests
mvn test
```

## Desktop UI

The default app window lets you:

- Add tasks with title, description, priority, due date, and assignee
- Filter by status or overdue state
- Sort by priority, due date, or creation order
- Start, complete, cancel, assign, reprioritize, and delete selected tasks
- See summary counts for total, status, and overdue tasks

Due dates should use `yyyy-MM-dd`.

## CLI Usage

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
```

When adding a task you will be prompted for:
- **Title** (required)
- **Priority** — `LOW`, `MEDIUM`, `HIGH`, or `CRITICAL` (default: `MEDIUM`)
- **Description** (optional)
- **Due date** in `yyyy-MM-dd` format (optional)

## Task Lifecycle

```
TODO ──► IN_PROGRESS ──► DONE
  │                       
  └──────────────────────► CANCELLED
```

Once a task reaches `DONE` or `CANCELLED` its status cannot be changed.

## Tech Stack

| Layer      | Technology              |
|------------|-------------------------|
| Language   | Java 11                 |
| Build      | Maven                   |
| Testing    | JUnit 5 (Jupiter)       |
| Storage    | In-memory (no database) |
