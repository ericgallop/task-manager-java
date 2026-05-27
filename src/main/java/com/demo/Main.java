package com.demo;

import java.util.List;
import java.util.Scanner;

public class Main {
    private static final TaskManager manager = new TaskManager();
    private static final Scanner scanner = new Scanner(System.in);

    public static void main(String[] args) {
        System.out.println("Task Manager CLI");
        System.out.println("================");

        boolean running = true;
        while (running) {
            printMenu();
            String input = scanner.nextLine().trim();

            switch (input) {
                case "1": addTask(); break;
                case "2": listTasks(); break;
                case "3": completeTask(); break;
                case "4": removeTask(); break;
                case "5": running = false; break;
                default: System.out.println("Invalid option.");
            }
        }

        System.out.println("Goodbye!");
        scanner.close();
    }

    private static void printMenu() {
        System.out.println("\n1. Add task");
        System.out.println("2. List tasks");
        System.out.println("3. Complete task");
        System.out.println("4. Remove task");
        System.out.println("5. Exit");
        System.out.print("Choose: ");
    }

    private static void addTask() {
        System.out.print("Task title: ");
        String title = scanner.nextLine().trim();
        if (title.isEmpty()) {
            System.out.println("Title cannot be empty.");
            return;
        }
        Task task = manager.addTask(title);
        System.out.println("Added: " + task);
    }

    private static void listTasks() {
        List<Task> tasks = manager.getAllTasks();
        if (tasks.isEmpty()) {
            System.out.println("No tasks.");
            return;
        }
        tasks.forEach(System.out::println);
    }

    private static void completeTask() {
        System.out.print("Task ID to complete: ");
        try {
            int id = Integer.parseInt(scanner.nextLine().trim());
            if (manager.completeTask(id)) {
                System.out.println("Task " + id + " completed.");
            } else {
                System.out.println("Task not found.");
            }
        } catch (NumberFormatException e) {
            System.out.println("Invalid ID.");
        }
    }

    private static void removeTask() {
        System.out.print("Task ID to remove: ");
        try {
            int id = Integer.parseInt(scanner.nextLine().trim());
            if (manager.removeTask(id)) {
                System.out.println("Task " + id + " removed.");
            } else {
                System.out.println("Task not found.");
            }
        } catch (NumberFormatException e) {
            System.out.println("Invalid ID.");
        }
    }
}
