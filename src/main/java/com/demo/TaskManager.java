package com.demo;

import java.util.List;
import java.util.Optional;

/**
 * Facade kept for backward compatibility — delegates to TaskService.
 */
public class TaskManager {
    private final TaskService service;

    public TaskManager() {
        this.service = new TaskService(new InMemoryTaskRepository());
    }

    public TaskManager(TaskService service) {
        this.service = service;
    }

    public Task addTask(String title) {
        return service.createTask(title);
    }

    public boolean completeTask(int id) {
        return service.completeTask(id);
    }

    public boolean removeTask(int id) {
        return service.deleteTask(id);
    }

    public List<Task> getAllTasks() {
        return service.getAllTasks();
    }

    public List<Task> getPendingTasks() {
        return service.getPendingTasks();
    }

    public TaskService getService() {
        return service;
    }
}
