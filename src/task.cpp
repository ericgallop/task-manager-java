#include "task.h"
#include "date_utils.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>

Task::Task(int id, const std::string& title, const std::string& created_date)
    : id_(id)
    , title_(title)
    , description_("")
    , priority_(Priority::MEDIUM)
    , status_(TaskStatus::TODO)
    , assignee_("")
    , due_date_("")
    , created_date_(created_date) {}

int Task::get_id() const { return id_; }
const std::string& Task::get_title() const { return title_; }
const std::string& Task::get_description() const { return description_; }
Priority Task::get_priority() const { return priority_; }
TaskStatus Task::get_status() const { return status_; }
const std::string& Task::get_assignee() const { return assignee_; }
const std::string& Task::get_due_date() const { return due_date_; }
const std::string& Task::get_created_date() const { return created_date_; }

void Task::set_title(const std::string& title) {
    // Validate non-blank
    bool blank = true;
    for (char c : title) {
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            blank = false;
            break;
        }
    }
    if (title.empty() || blank) {
        throw std::invalid_argument("Title cannot be blank");
    }
    title_ = title;
}

void Task::set_description(const std::string& description) { description_ = description; }
void Task::set_priority(Priority priority) { priority_ = priority; }

void Task::set_status(TaskStatus status) {
    if (is_terminal(status_)) {
        throw std::logic_error("Cannot change status of a terminal task");
    }
    status_ = status;
}

void Task::set_assignee(const std::string& assignee) { assignee_ = assignee; }
void Task::set_due_date(const std::string& due_date) { due_date_ = due_date; }

bool Task::is_completed() const { return status_ == TaskStatus::DONE; }

bool Task::is_overdue() const {
    if (due_date_.empty() || is_terminal(status_)) return false;
    std::string today = today_as_string();
    return today > due_date_;
}

TransitionResult Task::complete() {
    if (is_terminal(status_)) return TransitionResult::InvalidTransition;
    status_ = TaskStatus::DONE;
    return TransitionResult::Success;
}

TransitionResult Task::cancel() {
    if (is_terminal(status_)) return TransitionResult::InvalidTransition;
    status_ = TaskStatus::CANCELLED;
    return TransitionResult::Success;
}

TransitionResult Task::start_progress() {
    if (status_ != TaskStatus::TODO) return TransitionResult::InvalidTransition;
    status_ = TaskStatus::IN_PROGRESS;
    return TransitionResult::Success;
}

std::string Task::to_string() const {
    std::ostringstream oss;
    std::string due_part = due_date_.empty() ? "" : " (due: " + due_date_ + ")";
    std::string assignee_part = assignee_.empty() ? "" : " [@" + assignee_ + "]";

    oss << "[" << task_status_to_string(status_) << "] #" << id_
        << " " << std::left << std::setw(8) << ("[" + priority_to_string(priority_) + "]")
        << " " << title_ << due_part << assignee_part;
    return oss.str();
}
