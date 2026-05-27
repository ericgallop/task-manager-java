package com.demo;

public class TaskSummary {
    public final long total;
    public final long todo;
    public final long inProgress;
    public final long done;
    public final long cancelled;
    public final long overdue;

    public TaskSummary(long total, long todo, long inProgress, long done, long cancelled, long overdue) {
        this.total = total;
        this.todo = todo;
        this.inProgress = inProgress;
        this.done = done;
        this.cancelled = cancelled;
        this.overdue = overdue;
    }

    @Override
    public String toString() {
        return String.format(
                "Total: %d | TODO: %d | In Progress: %d | Done: %d | Cancelled: %d | Overdue: %d",
                total, todo, inProgress, done, cancelled, overdue);
    }
}
