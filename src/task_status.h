#ifndef TASK_STATUS_H
#define TASK_STATUS_H

enum class TaskStatus { TODO, IN_PROGRESS, DONE, CANCELLED };

inline bool is_terminal(TaskStatus s) {
    return s == TaskStatus::DONE || s == TaskStatus::CANCELLED;
}

#endif // TASK_STATUS_H
