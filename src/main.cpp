#include "task_service.h"
#include "task_store.h"
#include "priority.h"
#include <iostream>
#include <string>

void addTask(TaskService& service) {
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

int main() {
    TaskStore store;
    TaskService service(store);

    while (true) {
        std::cout << "\n=== Task Manager ===" << std::endl;
        std::cout << "1. Add Task" << std::endl;
        std::cout << "2. List Tasks" << std::endl;
        std::cout << "3. Start Task" << std::endl;
        std::cout << "4. Complete Task" << std::endl;
        std::cout << "5. Delete Task" << std::endl;
        std::cout << "6. Exit" << std::endl;
        std::cout << "Choice: ";

        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "1") {
            addTask(service);
        } else if (choice == "6") {
            std::cout << "Goodbye!" << std::endl;
            break;
        } else {
            std::cout << "Not implemented yet." << std::endl;
        }
    }

    return 0;
}
