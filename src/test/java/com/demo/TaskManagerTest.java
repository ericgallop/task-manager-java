package com.demo;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

public class TaskManagerTest {
    private TaskManager manager;

    @BeforeEach
    void setUp() {
        manager = new TaskManager();
    }

    @Test
    void addTask_createsTaskWithTitle() {
        Task task = manager.addTask("Write tests");
        assertEquals("Write tests", task.getTitle());
        assertFalse(task.isCompleted());
    }

    @Test
    void addTask_assignsIncrementingIds() {
        Task first = manager.addTask("First");
        Task second = manager.addTask("Second");
        assertEquals(first.getId() + 1, second.getId());
    }

    @Test
    void completeTask_marksTaskDone() {
        Task task = manager.addTask("Do something");
        assertTrue(manager.completeTask(task.getId()));
        assertTrue(task.isCompleted());
    }

    @Test
    void completeTask_returnsFalseForMissingId() {
        assertFalse(manager.completeTask(999));
    }

    @Test
    void removeTask_deletesTask() {
        Task task = manager.addTask("Delete me");
        assertTrue(manager.removeTask(task.getId()));
        assertTrue(manager.getAllTasks().isEmpty());
    }

    @Test
    void getPendingTasks_excludesCompleted() {
        Task t1 = manager.addTask("Done");
        Task t2 = manager.addTask("Pending");
        manager.completeTask(t1.getId());

        List<Task> pending = manager.getPendingTasks();
        assertEquals(1, pending.size());
        assertEquals(t2.getId(), pending.get(0).getId());
    }
}
