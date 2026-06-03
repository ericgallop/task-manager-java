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

// Result of a task lifecycle state transition attempt.
enum class TransitionResult {
    Success,          // Transition applied successfully
    NotFound,         // Task ID does not exist
    InvalidTransition // Transition not allowed from current state
};

inline std::string transition_result_to_string(TransitionResult r) {
    switch (r) {
        case TransitionResult::Success:           return "Success";
        case TransitionResult::NotFound:          return "NotFound";
        case TransitionResult::InvalidTransition: return "InvalidTransition";
    }
    return "Success";
}

#endif // TASK_STATUS_H
