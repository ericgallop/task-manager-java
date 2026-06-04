#ifndef TASK_SUMMARY_H
#define TASK_SUMMARY_H

// Header-only struct holding aggregate counts for the task list.
// Ported from: com.demo.TaskSummary (Java)
struct TaskSummary {
    int total = 0;
    int todo = 0;
    int in_progress = 0;
    int done = 0;
    int cancelled = 0;
    int overdue = 0;
};

#endif // TASK_SUMMARY_H
