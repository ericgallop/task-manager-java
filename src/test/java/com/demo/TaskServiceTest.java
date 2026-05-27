package com.demo;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.time.LocalDate;
import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

public class TaskServiceTest {
    private TaskService service;

    @BeforeEach
    void setUp() {
        service = new TaskService(new InMemoryTaskRepository());
    }

    @Test
    void createTask_defaultsMediumPriority() {
        Task task = service.createTask("Simple task");
        assertEquals(Priority.MEDIUM, task.getPriority());
    }

    @Test
    void createTask_withAllFields() {
        LocalDate due = LocalDate.now().plusDays(7);
        Task task = service.createTask("Full task", Priority.HIGH, "A description", due);
        assertEquals(Priority.HIGH, task.getPriority());
        assertEquals("A description", task.getDescription());
        assertEquals(due, task.getDueDate());
    }

    @Test
    void startTask_changesStatusToInProgress() {
        Task task = service.createTask("Task");
        assertTrue(service.startTask(task.getId()));
        assertEquals(TaskStatus.IN_PROGRESS, task.getStatus());
    }

    @Test
    void completeTask_fromInProgress() {
        Task task = service.createTask("Task");
        service.startTask(task.getId());
        assertTrue(service.completeTask(task.getId()));
        assertEquals(TaskStatus.DONE, task.getStatus());
    }

    @Test
    void cancelTask_fromTodo() {
        Task task = service.createTask("Task");
        assertTrue(service.cancelTask(task.getId()));
        assertEquals(TaskStatus.CANCELLED, task.getStatus());
    }

    @Test
    void startTask_failsForMissingId() {
        assertFalse(service.startTask(999));
    }

    @Test
    void assignTask_setsAssignee() {
        Task task = service.createTask("Task");
        assertTrue(service.assignTask(task.getId(), "alice"));
        assertEquals("alice", task.getAssignee());
    }

    @Test
    void updatePriority_changesPriority() {
        Task task = service.createTask("Task");
        service.updatePriority(task.getId(), Priority.CRITICAL);
        assertEquals(Priority.CRITICAL, task.getPriority());
    }

    @Test
    void getOverdueTasks_returnsOnlyOverdue() {
        Task overdue = service.createTask("Overdue", Priority.HIGH, null, LocalDate.now().minusDays(1));
        Task future = service.createTask("Future", Priority.LOW, null, LocalDate.now().plusDays(5));
        service.createTask("NoDue");

        List<Task> overdueList = service.getOverdueTasks();
        assertEquals(1, overdueList.size());
        assertEquals(overdue.getId(), overdueList.get(0).getId());
    }

    @Test
    void getTasksByPriority_filtersCorrectly() {
        service.createTask("High task", Priority.HIGH, null, null);
        service.createTask("Low task", Priority.LOW, null, null);
        service.createTask("Another high", Priority.HIGH, null, null);

        List<Task> highTasks = service.getTasksByPriority(Priority.HIGH);
        assertEquals(2, highTasks.size());
    }

    @Test
    void getSummary_countsCorrectly() {
        Task t1 = service.createTask("T1");
        Task t2 = service.createTask("T2");
        service.createTask("T3");

        service.startTask(t1.getId());
        service.completeTask(t2.getId());

        TaskSummary summary = service.getSummary();
        assertEquals(3, summary.total);
        assertEquals(1, summary.todo);
        assertEquals(1, summary.inProgress);
        assertEquals(1, summary.done);
    }

    @Test
    void deleteTask_removesTask() {
        Task task = service.createTask("Delete me");
        assertTrue(service.deleteTask(task.getId()));
        assertTrue(service.getAllTasks().isEmpty());
    }

    @Test
    void getTasksSortedByPriority_highestFirst() {
        service.createTask("Low", Priority.LOW, null, null);
        service.createTask("Critical", Priority.CRITICAL, null, null);
        service.createTask("Medium", Priority.MEDIUM, null, null);

        List<Task> sorted = service.getTasksSortedByPriority();
        assertEquals(Priority.CRITICAL, sorted.get(0).getPriority());
        assertEquals(Priority.LOW, sorted.get(sorted.size() - 1).getPriority());
    }
}
