#include "task_service.h"
#include "task_store.h"
#include "priority.h"
#include <iostream>
#include <string>
#include <vector>

// Helper: read an integer task ID from stdin. Returns -1 on invalid input.
static int readId(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    try {
        return std::stoi(line);
    } catch (...) {
        std::cout << "Invalid ID." << std::endl;
        return -1;
    }
}

static void addTask(TaskService& service) {
    std::string title, priority_str, description, due_date, assignee;

    std::cout << "Title: ";
    std::getline(std::cin, title);

    std::cout << "Priority [LOW/MEDIUM/HIGH/CRITICAL] (default MEDIUM): ";
    std::getline(std::cin, priority_str);
    Priority priority = priority_from_string(priority_str);

    std::cout << "Description (optional): ";
    std::getline(std::cin, description);

    std::cout << "Due date [yyyy-MM-dd] (optional): ";
    std::getline(std::cin, due_date);

    std::cout << "Assignee (optional): ";
    std::getline(std::cin, assignee);

    auto result = service.createTask(title, priority, description, due_date, assignee);
    if (result.has_value()) {
        std::cout << "Created: " << result->to_string() << std::endl;
    }
}

static void listTasks(TaskService& service) {
    auto tasks = service.getAllTasks();
    if (tasks.empty()) {
        std::cout << "No tasks." << std::endl;
        return;
    }
    for (const auto& task : tasks) {
        std::cout << task.to_string() << std::endl;
    }
}

static void startTask(TaskService& service) {
    int id = readId("Task ID to start: ");
    if (id < 0) return;
    auto result = service.startTask(id);
    switch (result) {
        case TransitionResult::Success:
            std::cout << "Task " << id << " is now IN_PROGRESS." << std::endl;
            break;
        case TransitionResult::NotFound:
            std::cout << "Task not found." << std::endl;
            break;
        case TransitionResult::InvalidTransition:
            std::cout << "Cannot start task — task must be in TODO status." << std::endl;
            break;
    }
}

static void completeTask(TaskService& service) {
    int id = readId("Task ID to complete: ");
    if (id < 0) return;
    auto result = service.completeTask(id);
    switch (result) {
        case TransitionResult::Success:
            std::cout << "Task " << id << " completed." << std::endl;
            break;
        case TransitionResult::NotFound:
            std::cout << "Task not found." << std::endl;
            break;
        case TransitionResult::InvalidTransition:
            std::cout << "Cannot complete task — task is already completed or cancelled." << std::endl;
            break;
    }
}

static void cancelTask(TaskService& service) {
    int id = readId("Task ID to cancel: ");
    if (id < 0) return;
    auto result = service.cancelTask(id);
    switch (result) {
        case TransitionResult::Success:
            std::cout << "Task " << id << " cancelled." << std::endl;
            break;
        case TransitionResult::NotFound:
            std::cout << "Task not found." << std::endl;
            break;
        case TransitionResult::InvalidTransition:
            std::cout << "Cannot cancel task — task is already completed or cancelled." << std::endl;
            break;
    }
}

static void assignTask(TaskService& service) {
    int id = readId("Task ID to assign: ");
    if (id < 0) return;
    std::cout << "Assignee name: ";
    std::string assignee;
    std::getline(std::cin, assignee);
    if (service.assignTask(id, assignee)) {
        if (assignee.empty())
            std::cout << "Cleared assignee from task " << id << std::endl;
        else
            std::cout << "Assigned task " << id << " to " << assignee << std::endl;
    } else {
        std::cout << "Task not found." << std::endl;
    }
}

static void setPriority(TaskService& service) {
    int id = readId("Task ID: ");
    if (id < 0) return;
    std::cout << "New priority [LOW/MEDIUM/HIGH/CRITICAL]: ";
    std::string pstr;
    std::getline(std::cin, pstr);
    Priority p = priority_from_string(pstr);
    if (service.updatePriority(id, p))
        std::cout << "Priority updated." << std::endl;
    else
        std::cout << "Task not found." << std::endl;
}

static void removeTask(TaskService& service) {
    int id = readId("Task ID to remove: ");
    if (id < 0) return;

    // Check that the task exists before prompting for confirmation
    auto task = service.getTask(id);
    if (!task.has_value()) {
        std::cout << "Task not found." << std::endl;
        return;
    }

    // Confirmation prompt
    std::cout << "Are you sure you want to delete task #" << id << "? (y/n): ";
    std::string answer;
    std::getline(std::cin, answer);

    if (answer == "y" || answer == "Y") {
        service.deleteTask(id);
        std::cout << "Task " << id << " removed." << std::endl;
    } else {
        std::cout << "Delete cancelled." << std::endl;
    }
}

static void printMenu() {
    std::cout << "\n 1. Add task" << std::endl;
    std::cout << " 2. List all tasks" << std::endl;
    std::cout << " 3. Start task" << std::endl;
    std::cout << " 4. Complete task" << std::endl;
    std::cout << " 5. Cancel task" << std::endl;
    std::cout << " 6. Assign task" << std::endl;
    std::cout << " 7. Set priority" << std::endl;
    std::cout << " 8. Remove task" << std::endl;
    std::cout << " 0. Exit" << std::endl;
    std::cout << "Choose: ";
}

int main() {
    TaskStore store;
    TaskService service(store);

    std::cout << "Task Manager CLI" << std::endl;
    std::cout << "================" << std::endl;

    bool running = true;
    while (running) {
        printMenu();
        std::string choice;
        std::getline(std::cin, choice);

        // Trim whitespace
        auto start = choice.find_first_not_of(" \t");
        if (start != std::string::npos) {
            choice = choice.substr(start, choice.find_last_not_of(" \t") - start + 1);
        }

        if (choice == "1")       addTask(service);
        else if (choice == "2")  listTasks(service);
        else if (choice == "3")  startTask(service);
        else if (choice == "4")  completeTask(service);
        else if (choice == "5")  cancelTask(service);
        else if (choice == "6")  assignTask(service);
        else if (choice == "7")  setPriority(service);
        else if (choice == "8")  removeTask(service);
        else if (choice == "0")  running = false;
        else                     std::cout << "Invalid option." << std::endl;
    }

    std::cout << "Goodbye!" << std::endl;
    return 0;
}
