package com.demo;

import java.time.LocalDate;
import java.time.format.DateTimeFormatter;

public class Task {
    private static final DateTimeFormatter DATE_FMT = DateTimeFormatter.ofPattern("yyyy-MM-dd");

    private final int id;
    private String title;
    private String description;
    private TaskStatus status;
    private Priority priority;
    private LocalDate dueDate;
    private String assignee;
    private final LocalDate createdDate;

    public Task(int id, String title) {
        this.id = id;
        this.title = title;
        this.description = "";
        this.status = TaskStatus.TODO;
        this.priority = Priority.MEDIUM;
        this.createdDate = LocalDate.now();
    }

    public int getId() { return id; }
    public String getTitle() { return title; }
    public String getDescription() { return description; }
    public TaskStatus getStatus() { return status; }
    public Priority getPriority() { return priority; }
    public LocalDate getDueDate() { return dueDate; }
    public String getAssignee() { return assignee; }
    public LocalDate getCreatedDate() { return createdDate; }

    public void setTitle(String title) {
        if (title == null || title.isBlank()) throw new IllegalArgumentException("Title cannot be blank");
        this.title = title;
    }

    public void setDescription(String description) { this.description = description != null ? description : ""; }
    public void setPriority(Priority priority) { this.priority = priority; }
    public void setDueDate(LocalDate dueDate) { this.dueDate = dueDate; }
    public void setAssignee(String assignee) { this.assignee = assignee; }

    public void setStatus(TaskStatus status) {
        if (this.status.isTerminal()) {
            throw new IllegalStateException("Cannot change status of a terminal task");
        }
        this.status = status;
    }

    public boolean isCompleted() { return status == TaskStatus.DONE; }
    public boolean isOverdue() { return dueDate != null && LocalDate.now().isAfter(dueDate) && !status.isTerminal(); }

    public void complete() { this.status = TaskStatus.DONE; }
    public void cancel() { this.status = TaskStatus.CANCELLED; }
    public void startProgress() {
        if (status != TaskStatus.TODO) throw new IllegalStateException("Task must be in TODO to start");
        this.status = TaskStatus.IN_PROGRESS;
    }

    @Override
    public String toString() {
        String duePart = dueDate != null ? " (due: " + dueDate.format(DATE_FMT) + ")" : "";
        String assigneePart = assignee != null ? " [@" + assignee + "]" : "";
        String overduePart = isOverdue() ? " ⚠ OVERDUE" : "";
        return String.format("[%s] #%d %-8s %s%s%s%s",
                status, id, "[" + priority + "]", title, duePart, assigneePart, overduePart);
    }
}
