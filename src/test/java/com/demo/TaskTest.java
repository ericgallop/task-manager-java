package com.demo;

import org.junit.jupiter.api.Test;

import java.time.LocalDate;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Direct unit tests for {@link Task#isOverdue()} model method.
 * Validates overdue logic independent of the service layer.
 *
 * Overdue definition: a task is overdue when ALL THREE conditions hold:
 *   1. dueDate is non-null (task has a due date)
 *   2. today is strictly after dueDate (due date is in the past)
 *   3. status is NOT terminal (not DONE or CANCELLED)
 */
public class TaskTest {

    // ---- AC1: Past due date + non-terminal status → overdue ----

    @Test
    void isOverdue_todoWithPastDueDate_returnsTrue() {
        // AC1: TODO task with due date in the past is overdue
        Task task = new Task(1, "Overdue todo task");
        task.setDueDate(LocalDate.now().minusDays(1));
        // status defaults to TODO

        assertTrue(task.isOverdue(),
                "A TODO task with a past due date should be overdue");
    }

    @Test
    void isOverdue_inProgressWithPastDueDate_returnsTrue() {
        // AC1: IN_PROGRESS task with due date in the past is overdue
        Task task = new Task(2, "Overdue in-progress task");
        task.setDueDate(LocalDate.now().minusDays(5));
        task.startProgress(); // moves from TODO → IN_PROGRESS

        assertTrue(task.isOverdue(),
                "An IN_PROGRESS task with a past due date should be overdue");
    }

    @Test
    void isOverdue_todoWithFarPastDueDate_returnsTrue() {
        // AC1: Edge — task overdue by many days is still overdue
        Task task = new Task(3, "Very overdue task");
        task.setDueDate(LocalDate.of(2020, 1, 1));

        assertTrue(task.isOverdue(),
                "A TODO task with a due date far in the past should be overdue");
    }

    // ---- AC2: Past due date + terminal status → NOT overdue ----

    @Test
    void isOverdue_doneWithPastDueDate_returnsFalse() {
        // AC2: DONE task with due date in the past is NOT overdue
        Task task = new Task(4, "Completed late task");
        task.setDueDate(LocalDate.now().minusDays(3));
        task.complete(); // moves to DONE

        assertFalse(task.isOverdue(),
                "A DONE task should NOT be overdue even if due date is in the past");
    }

    @Test
    void isOverdue_cancelledWithPastDueDate_returnsFalse() {
        // AC2: CANCELLED task with due date in the past is NOT overdue
        Task task = new Task(5, "Cancelled late task");
        task.setDueDate(LocalDate.now().minusDays(7));
        task.cancel(); // moves to CANCELLED

        assertFalse(task.isOverdue(),
                "A CANCELLED task should NOT be overdue even if due date is in the past");
    }

    // ---- AC3: No due date → never overdue ----

    @Test
    void isOverdue_todoWithNoDueDate_returnsFalse() {
        // AC3: A TODO task with no due date is never overdue
        Task task = new Task(6, "No due date task");
        // dueDate is null by default

        assertNull(task.getDueDate(), "Precondition: dueDate should be null");
        assertFalse(task.isOverdue(),
                "A task with no due date should never be overdue");
    }

    @Test
    void isOverdue_inProgressWithNoDueDate_returnsFalse() {
        // AC3: An IN_PROGRESS task with no due date is never overdue
        Task task = new Task(7, "In progress no due date");
        task.startProgress();

        assertNull(task.getDueDate(), "Precondition: dueDate should be null");
        assertFalse(task.isOverdue(),
                "An IN_PROGRESS task with no due date should never be overdue");
    }

    @Test
    void isOverdue_doneWithNoDueDate_returnsFalse() {
        // AC3: A DONE task with no due date is never overdue
        Task task = new Task(8, "Done no due date");
        task.complete();

        assertFalse(task.isOverdue(),
                "A DONE task with no due date should never be overdue");
    }

    @Test
    void isOverdue_cancelledWithNoDueDate_returnsFalse() {
        // AC3: A CANCELLED task with no due date is never overdue
        Task task = new Task(9, "Cancelled no due date");
        task.cancel();

        assertFalse(task.isOverdue(),
                "A CANCELLED task with no due date should never be overdue");
    }

    // ---- Edge case: Task due today → NOT overdue (strict 'after' semantics) ----

    @Test
    void isOverdue_todoDueToday_returnsFalse() {
        // Edge: A task due today is NOT overdue (Java's isAfter is strict)
        Task task = new Task(10, "Due today task");
        task.setDueDate(LocalDate.now());

        assertFalse(task.isOverdue(),
                "A task due today should NOT be overdue (strict after semantics)");
    }

    @Test
    void isOverdue_inProgressDueToday_returnsFalse() {
        // Edge: An IN_PROGRESS task due today is NOT overdue
        Task task = new Task(11, "In progress due today");
        task.setDueDate(LocalDate.now());
        task.startProgress();

        assertFalse(task.isOverdue(),
                "An IN_PROGRESS task due today should NOT be overdue");
    }

    // ---- Edge case: Future due date → NOT overdue ----

    @Test
    void isOverdue_todoWithFutureDueDate_returnsFalse() {
        // Edge: A task with a future due date is NOT overdue
        Task task = new Task(12, "Future task");
        task.setDueDate(LocalDate.now().plusDays(10));

        assertFalse(task.isOverdue(),
                "A TODO task with a future due date should NOT be overdue");
    }

    @Test
    void isOverdue_inProgressWithFutureDueDate_returnsFalse() {
        // Edge: An IN_PROGRESS task with a future due date is NOT overdue
        Task task = new Task(13, "In progress future");
        task.setDueDate(LocalDate.now().plusDays(30));
        task.startProgress();

        assertFalse(task.isOverdue(),
                "An IN_PROGRESS task with a future due date should NOT be overdue");
    }

    // ---- Edge case: Terminal status with future due date → NOT overdue ----

    @Test
    void isOverdue_doneWithFutureDueDate_returnsFalse() {
        // Edge: A DONE task with a future due date is NOT overdue
        Task task = new Task(14, "Done early");
        task.setDueDate(LocalDate.now().plusDays(5));
        task.complete();

        assertFalse(task.isOverdue(),
                "A DONE task with a future due date should NOT be overdue");
    }

    @Test
    void isOverdue_cancelledWithFutureDueDate_returnsFalse() {
        // Edge: A CANCELLED task with a future due date is NOT overdue
        Task task = new Task(15, "Cancelled early");
        task.setDueDate(LocalDate.now().plusDays(5));
        task.cancel();

        assertFalse(task.isOverdue(),
                "A CANCELLED task with a future due date should NOT be overdue");
    }

    // ---- Edge case: Due date exactly yesterday → overdue ----

    @Test
    void isOverdue_todoDueYesterday_returnsTrue() {
        // Boundary: due date is exactly yesterday — should be overdue
        Task task = new Task(16, "Due yesterday");
        task.setDueDate(LocalDate.now().minusDays(1));

        assertTrue(task.isOverdue(),
                "A TODO task due yesterday should be overdue");
    }
}
