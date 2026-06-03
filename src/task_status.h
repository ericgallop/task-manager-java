#ifndef TASK_STATUS_H
#define TASK_STATUS_H

#include <string>

enum class TaskStatus { TODO, IN_PROGRESS, DONE, CANCELLED };

inline bool is_terminal(TaskStatus s) {
    return s == TaskStatus::DONE || s == TaskStatus::CANCELLED;
}

inline std::string task_status_to_string(TaskStatus s) {
    switch (s) {
        case TaskStatus::TODO:        return "TODO";
        case TaskStatus::IN_PROGRESS: return "IN_PROGRESS";
        case TaskStatus::DONE:        return "DONE";
        case TaskStatus::CANCELLED:   return "CANCELLED";
    }
    return "TODO";
}

#endif // TASK_STATUS_H
