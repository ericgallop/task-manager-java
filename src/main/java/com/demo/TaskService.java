package com.demo;

import java.time.LocalDate;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

public class TaskService {
    private final TaskRepository repository;

    public TaskService(TaskRepository repository) {
        this.repository = repository;
    }

    public Task createTask(String title) {
        return createTask(title, Priority.MEDIUM, null, null);
    }

    public Task createTask(String title, Priority priority, String description, LocalDate dueDate) {
        Task task = new Task(repository.nextId(), title);
        task.setPriority(priority);
        task.setDescription(description);
        task.setDueDate(dueDate);
        return repository.save(task);
    }

    public Optional<Task> getTask(int id) {
        return repository.findById(id);
    }

    public List<Task> getAllTasks() {
        return repository.findAll();
    }

    public List<Task> getPendingTasks() {
        return repository.findByStatus(TaskStatus.TODO);
    }

    public List<Task> getInProgressTasks() {
        return repository.findByStatus(TaskStatus.IN_PROGRESS);
    }

    public List<Task> getCompletedTasks() {
        return repository.findByStatus(TaskStatus.DONE);
    }

    public List<Task> getOverdueTasks() {
        return repository.findAll().stream()
                .filter(Task::isOverdue)
                .collect(Collectors.toList());
    }

    public List<Task> getCancelledTasks() {
        return repository.findByStatus(TaskStatus.CANCELLED);
    }

    public List<Task> getFilteredTasks(String filter) {
        switch (filter) {
            case "all":
                return getTasksSortedByPriority();
            case "todo":
                return repository.findByStatus(TaskStatus.TODO);
            case "in_progress":
                return repository.findByStatus(TaskStatus.IN_PROGRESS);
            case "done":
                return repository.findByStatus(TaskStatus.DONE);
            case "cancelled":
                return repository.findByStatus(TaskStatus.CANCELLED);
            case "overdue":
                return repository.findAll().stream()
                        .filter(Task::isOverdue)
                        .collect(Collectors.toList());
            default:
                return new ArrayList<>();
        }
    }

    public List<Task> getTasksByPriority(Priority priority) {
        return repository.findByPriority(priority);
    }

    public List<Task> getTasksSortedByPriority() {
        return repository.findAll().stream()
                .sorted(Comparator.comparing(Task::getPriority).reversed())
                .collect(Collectors.toList());
    }

    public List<Task> getTasksSortedByDueDate() {
        Comparator<Task> dueDateComparator = (left, right) -> {
            boolean leftHas = left.getDueDate() != null;
            boolean rightHas = right.getDueDate() != null;
            if (leftHas && rightHas) {
                return left.getDueDate().compareTo(right.getDueDate());
            }
            if (leftHas) {
                return -1;
            }
            if (rightHas) {
                return 1;
            }
            return Integer.compare(left.getId(), right.getId());
        };
        return repository.findAll().stream()
                .sorted(dueDateComparator)
                .collect(Collectors.toList());
    }

    public List<Task> getTasksSortedByCreationOrder() {
        return repository.findAll().stream()
                .sorted(Comparator.comparingInt(Task::getId))
                .collect(Collectors.toList());
    }

    public boolean startTask(int id) {
        return repository.findById(id).map(t -> {
            t.startProgress();
            return true;
        }).orElse(false);
    }

    public boolean completeTask(int id) {
        return repository.findById(id).map(t -> {
            t.complete();
            return true;
        }).orElse(false);
    }

    public boolean cancelTask(int id) {
        return repository.findById(id).map(t -> {
            t.cancel();
            return true;
        }).orElse(false);
    }

    public boolean assignTask(int id, String assignee) {
        return repository.findById(id).map(t -> {
            t.setAssignee(assignee);
            return true;
        }).orElse(false);
    }

    public boolean updatePriority(int id, Priority priority) {
        return repository.findById(id).map(t -> {
            t.setPriority(priority);
            return true;
        }).orElse(false);
    }

    public boolean deleteTask(int id) {
        return repository.delete(id);
    }

    public TaskSummary getSummary() {
        List<Task> all = repository.findAll();
        long todo = all.stream().filter(t -> t.getStatus() == TaskStatus.TODO).count();
        long inProgress = all.stream().filter(t -> t.getStatus() == TaskStatus.IN_PROGRESS).count();
        long done = all.stream().filter(t -> t.getStatus() == TaskStatus.DONE).count();
        long cancelled = all.stream().filter(t -> t.getStatus() == TaskStatus.CANCELLED).count();
        long overdue = all.stream().filter(Task::isOverdue).count();
        return new TaskSummary(all.size(), todo, inProgress, done, cancelled, overdue);
    }
}
