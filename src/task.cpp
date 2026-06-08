#include "task.h"

#include <ctime>

namespace {

/// Returns today's date as a "YYYY-MM-DD" string (local time).
/// Mirrors Java's LocalDate.now() used in the Task constructor.
std::string today_string() {
    auto now = std::time(nullptr);
    auto* tm = std::localtime(&now);
    char buf[11]; // "YYYY-MM-DD\0"
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", tm);
    return std::string(buf);
}

} // anonymous namespace

Task::Task(int id, const std::string& title)
    : id_(id)
    , title_(title)
    , description_("")
    , status_(TaskStatus::TODO)
    , priority_(Priority::MEDIUM)
    , due_date_("")
    , assignee_("")
    , created_date_(today_string()) {}
