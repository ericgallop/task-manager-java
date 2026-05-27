package com.demo;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.stream.Collectors;

public class InMemoryTaskRepository implements TaskRepository {
    private final Map<Integer, Task> store = new LinkedHashMap<>();
    private final AtomicInteger idSeq = new AtomicInteger(1);

    @Override
    public Task save(Task task) {
        store.put(task.getId(), task);
        return task;
    }

    @Override
    public Optional<Task> findById(int id) {
        return Optional.ofNullable(store.get(id));
    }

    @Override
    public List<Task> findAll() {
        return new ArrayList<>(store.values());
    }

    @Override
    public List<Task> findByStatus(TaskStatus status) {
        return store.values().stream()
                .filter(t -> t.getStatus() == status)
                .collect(Collectors.toList());
    }

    @Override
    public List<Task> findByPriority(Priority priority) {
        return store.values().stream()
                .filter(t -> t.getPriority() == priority)
                .collect(Collectors.toList());
    }

    @Override
    public List<Task> findByAssignee(String assignee) {
        return store.values().stream()
                .filter(t -> assignee.equals(t.getAssignee()))
                .collect(Collectors.toList());
    }

    @Override
    public boolean delete(int id) {
        return store.remove(id) != null;
    }

    @Override
    public int nextId() {
        return idSeq.getAndIncrement();
    }
}
