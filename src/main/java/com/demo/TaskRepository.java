package com.demo;

import java.util.List;
import java.util.Optional;

public interface TaskRepository {
    Task save(Task task);
    Optional<Task> findById(int id);
    List<Task> findAll();
    List<Task> findByStatus(TaskStatus status);
    List<Task> findByPriority(Priority priority);
    List<Task> findByAssignee(String assignee);
    boolean delete(int id);
    int nextId();
}
