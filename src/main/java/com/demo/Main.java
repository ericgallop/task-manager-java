package com.demo;

import java.time.LocalDate;
import java.util.List;
import java.util.Scanner;

public class Main {
    private static final TaskService service = new TaskService(new InMemoryTaskRepository());
    private static final Scanner scanner = new Scanner(System.in);

    public static void main(String[] args) {
        runCli();
    }

    private static void runCli() {
        System.out.println("Task Manager CLI");
        System.out.println("================");

        boolean running = true;
        while (running) {
            printMenu();
            String input = scanner.nextLine().trim();

            switch (input) {
                case "1":  addTask();          break;
                case "2":  listTasks();        break;
                case "3":  startTask();        break;
                case "4":  completeTask();     break;
                case "5":  cancelTask();       break;
                case "6":  assignTask();       break;
                case "7":  setPriority();      break;
                case "8":  removeTask();       break;
                case "9":  showSummary();      break;
                case "10": showOverdue();      break;
                case "11": filterTasks();      break;
                case "0":  running = false;    break;
                default:   System.out.println("Invalid option.");
            }
        }

        System.out.println("Goodbye!");
        scanner.close();
    }

    private static void printMenu() {
        System.out.println("\n 1. Add task");
        System.out.println(" 2. List all tasks");
        System.out.println(" 3. Start task");
        System.out.println(" 4. Complete task");
        System.out.println(" 5. Cancel task");
        System.out.println(" 6. Assign task");
        System.out.println(" 7. Set priority");
        System.out.println(" 8. Remove task");
        System.out.println(" 9. Summary");
        System.out.println("10. Show overdue");
        System.out.println("11. Filter tasks by status");
        System.out.println(" 0. Exit");
        System.out.print("Choose: ");
    }

    private static void addTask() {
        System.out.print("Title: ");
        String title = scanner.nextLine().trim();
        if (title.isEmpty()) { System.out.println("Title cannot be empty."); return; }

        System.out.print("Priority [LOW/MEDIUM/HIGH/CRITICAL] (default MEDIUM): ");
        Priority priority = Priority.fromString(scanner.nextLine().trim());

        System.out.print("Description (optional): ");
        String desc = scanner.nextLine().trim();

        System.out.print("Due date [yyyy-MM-dd] (optional): ");
        String dateStr = scanner.nextLine().trim();
        LocalDate due = null;
        if (!dateStr.isEmpty()) {
            try { due = LocalDate.parse(dateStr); }
            catch (Exception e) { System.out.println("Invalid date format — skipping due date."); }
        }

        Task task = service.createTask(title, priority, desc.isEmpty() ? null : desc, due);
        System.out.println("Created: " + task);
    }

    private static void listTasks() {
        System.out.println("Sort by:");
        System.out.println("  1. Priority (highest first)");
        System.out.println("  2. Due date (earliest first)");
        System.out.println("  3. Creation order");
        System.out.print("Choose: ");

        String choice = scanner.nextLine().trim();
        List<Task> tasks;
        switch (choice) {
            case "1":
                tasks = service.getTasksSortedByPriority();
                break;
            case "2":
                tasks = service.getTasksSortedByDueDate();
                break;
            case "3":
                tasks = service.getTasksSortedByCreationOrder();
                break;
            default:
                System.out.println("Invalid choice, sorting by priority.");
                tasks = service.getTasksSortedByPriority();
                break;
        }

        if (tasks.isEmpty()) { System.out.println("No tasks."); return; }
        tasks.forEach(System.out::println);
    }

    private static void startTask() {
        int id = readId("Task ID to start: ");
        if (id < 0) return;
        if (service.startTask(id)) System.out.println("Task " + id + " is now IN_PROGRESS.");
        else System.out.println("Task not found or already started.");
    }

    private static void completeTask() {
        int id = readId("Task ID to complete: ");
        if (id < 0) return;
        if (service.completeTask(id)) System.out.println("Task " + id + " completed.");
        else System.out.println("Task not found.");
    }

    private static void cancelTask() {
        int id = readId("Task ID to cancel: ");
        if (id < 0) return;
        if (service.cancelTask(id)) System.out.println("Task " + id + " cancelled.");
        else System.out.println("Task not found.");
    }

    private static void assignTask() {
        int id = readId("Task ID to assign: ");
        if (id < 0) return;
        System.out.print("Assignee name: ");
        String assignee = scanner.nextLine().trim();
        if (service.assignTask(id, assignee)) System.out.println("Assigned task " + id + " to " + assignee);
        else System.out.println("Task not found.");
    }

    private static void setPriority() {
        int id = readId("Task ID: ");
        if (id < 0) return;
        System.out.print("New priority [LOW/MEDIUM/HIGH/CRITICAL]: ");
        Priority p = Priority.fromString(scanner.nextLine().trim());
        if (service.updatePriority(id, p)) System.out.println("Priority updated.");
        else System.out.println("Task not found.");
    }

    private static void removeTask() {
        int id = readId("Task ID to remove: ");
        if (id < 0) return;
        if (service.deleteTask(id)) System.out.println("Task " + id + " removed.");
        else System.out.println("Task not found.");
    }

    private static void showSummary() {
        System.out.println(service.getSummary());
    }

    private static void showOverdue() {
        List<Task> overdue = service.getOverdueTasks();
        if (overdue.isEmpty()) System.out.println("No overdue tasks.");
        else overdue.forEach(System.out::println);
    }

    private static void filterTasks() {
        System.out.println("Filter by:");
        System.out.println("  1. All tasks");
        System.out.println("  2. Todo");
        System.out.println("  3. In progress");
        System.out.println("  4. Done");
        System.out.println("  5. Cancelled");
        System.out.println("  6. Overdue");
        System.out.print("Choose filter: ");

        String choice = scanner.nextLine().trim();
        String filter;
        switch (choice) {
            case "1": filter = "all";         break;
            case "2": filter = "todo";        break;
            case "3": filter = "in_progress"; break;
            case "4": filter = "done";        break;
            case "5": filter = "cancelled";   break;
            case "6": filter = "overdue";     break;
            default:
                System.out.println("Invalid filter option.");
                return;
        }

        List<Task> tasks = service.getFilteredTasks(filter);
        if (tasks.isEmpty()) {
            System.out.println("No tasks match the selected filter.");
        } else {
            tasks.forEach(System.out::println);
        }
    }

    private static int readId(String prompt) {
        System.out.print(prompt);
        try {
            return Integer.parseInt(scanner.nextLine().trim());
        } catch (NumberFormatException e) {
            System.out.println("Invalid ID.");
            return -1;
        }
    }
}
