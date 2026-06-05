package com.demo;

import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.*;

/**
 * End-to-end tests for the CLI menu loop (10 menu operations + exit).
 * Launches Main as a subprocess (no --cli flag needed) and pipes stdin commands.
 */
public class CliE2ETest {

    private static String classpath;

    @BeforeAll
    static void resolveClasspath() {
        Path targetClasses = Paths.get("target", "classes").toAbsolutePath();
        classpath = targetClasses.toString();
    }

    /**
     * Helper: launches the CLI with the given stdin input and returns all stdout.
     * No --cli flag — the app goes directly into CLI mode.
     */
    private String runCli(String stdinInput) throws Exception {
        ProcessBuilder pb = new ProcessBuilder(
                "java", "-cp", classpath, "com.demo.Main"
        );
        pb.redirectErrorStream(true);
        pb.directory(Paths.get(".").toAbsolutePath().toFile());

        Process process = pb.start();

        // Write all input then close stdin
        try (OutputStream os = process.getOutputStream()) {
            os.write(stdinInput.getBytes(StandardCharsets.UTF_8));
            os.flush();
        }

        // Read all output
        String output;
        try (BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8))) {
            output = reader.lines().collect(Collectors.joining("\n"));
        }

        boolean finished = process.waitFor(10, TimeUnit.SECONDS);
        assertTrue(finished, "CLI process did not terminate in time");
        return output;
    }

    // ---- AC4: Exit with goodbye message ----

    @Test
    void exit_printsGoodbye() throws Exception {
        String input = "0\n";
        String output = runCli(input);

        assertTrue(output.contains("Task Manager CLI"), "Should show CLI header");
        assertTrue(output.contains("================"), "Should show header separator");
        assertTrue(output.contains("Goodbye!"), "Should print Goodbye! on exit");
    }

    // ---- Menu display ----

    @Test
    void menu_showsAllTenOptions() throws Exception {
        String input = "0\n";
        String output = runCli(input);

        assertTrue(output.contains("1. Add task"), "Menu should show option 1");
        assertTrue(output.contains("2. List all tasks"), "Menu should show option 2");
        assertTrue(output.contains("3. Start task"), "Menu should show option 3");
        assertTrue(output.contains("4. Complete task"), "Menu should show option 4");
        assertTrue(output.contains("5. Cancel task"), "Menu should show option 5");
        assertTrue(output.contains("6. Assign task"), "Menu should show option 6");
        assertTrue(output.contains("7. Set priority"), "Menu should show option 7");
        assertTrue(output.contains("8. Remove task"), "Menu should show option 8");
        assertTrue(output.contains("9. Summary"), "Menu should show option 9");
        assertTrue(output.contains("10. Show overdue"), "Menu should show option 10");
        assertTrue(output.contains("0. Exit"), "Menu should show option 0");
        assertFalse(output.contains("11."), "Menu should NOT have option 11");
        assertTrue(output.contains("Choose:"), "Menu should show Choose: prompt");
    }

    @Test
    void invalidOption_showsInvalidMessage() throws Exception {
        String input = "99\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Invalid option."),
                "Should display 'Invalid option.' for unrecognized input");
    }

    // ---- Option 1: Add task ----

    @Test
    void addTask_createsTaskAndShowsConfirmation() throws Exception {
        // title, priority, description, due date, then exit
        String input = "1\nBuy groceries\nHIGH\nWeekly shopping\n\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Created:"), "Should confirm task creation");
        assertTrue(output.contains("Buy groceries"), "Output should include task title");
    }

    @Test
    void addTask_emptyTitle_showsError() throws Exception {
        // empty title, then exit
        String input = "1\n\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Title cannot be empty."),
                "Should reject empty title");
    }

    @Test
    void addTask_invalidDate_skipsDate() throws Exception {
        // title, priority, description, bad date, then exit
        String input = "1\nTest task\nMEDIUM\n\nnot-a-date\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Invalid date format"),
                "Should warn about invalid date format");
        assertTrue(output.contains("Created:"), "Should still create the task");
    }

    @Test
    void addTask_withValidDueDate_createsTask() throws Exception {
        String futureDate = LocalDate.now().plusDays(7).format(DateTimeFormatter.ISO_LOCAL_DATE);
        String input = "1\nDated task\nLOW\nSome desc\n" + futureDate + "\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Created:"), "Should confirm creation");
        assertTrue(output.contains("Dated task"), "Should include task title");
    }

    // ---- Option 2: List all tasks ----

    @Test
    void listTasks_empty_showsNoTasks() throws Exception {
        String input = "2\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("No tasks."),
                "Should display 'No tasks.' when no tasks exist");
    }

    @Test
    void listTasks_afterAdding_showsTask() throws Exception {
        // Add a task, then list, then exit
        String input = "1\nMy task\nHIGH\n\n\n2\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("My task"), "Listed tasks should include the created task");
        assertFalse(output.contains("No tasks."), "Should not say 'No tasks.' after adding one");
    }

    // ---- Option 3: Start task ----

    @Test
    void startTask_success() throws Exception {
        // Add task (id=1), then start it, then exit
        String input = "1\nStart me\nMEDIUM\n\n\n3\n1\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Task 1 is now IN_PROGRESS."),
                "Should confirm task started");
    }

    @Test
    void startTask_notFound() throws Exception {
        String input = "3\n999\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Task not found or already started."),
                "Should report task not found for nonexistent ID");
    }

    @Test
    void startTask_invalidId() throws Exception {
        String input = "3\nabc\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Invalid ID."),
                "Should display 'Invalid ID.' for non-numeric input");
    }

    // ---- Option 4: Complete task ----

    @Test
    void completeTask_success() throws Exception {
        // Add task, then complete it, then exit
        String input = "1\nComplete me\nMEDIUM\n\n\n4\n1\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Task 1 completed."),
                "Should confirm task completed");
    }

    @Test
    void completeTask_notFound() throws Exception {
        String input = "4\n999\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Task not found."),
                "Should report task not found");
    }

    // ---- Option 5: Cancel task ----

    @Test
    void cancelTask_success() throws Exception {
        // Add task, then cancel it, then exit
        String input = "1\nCancel me\nMEDIUM\n\n\n5\n1\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Task 1 cancelled."),
                "Should confirm task cancelled");
    }

    @Test
    void cancelTask_notFound() throws Exception {
        String input = "5\n999\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Task not found."),
                "Should report task not found");
    }

    // ---- Option 6: Assign task ----

    @Test
    void assignTask_success() throws Exception {
        // Add task, then assign it, then exit
        String input = "1\nAssign me\nMEDIUM\n\n\n6\n1\nalice\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Assigned task 1 to alice"),
                "Should confirm assignment");
    }

    @Test
    void assignTask_notFound() throws Exception {
        String input = "6\n999\nbob\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Task not found."),
                "Should report task not found");
    }

    // ---- Option 7: Set priority ----

    @Test
    void setPriority_success() throws Exception {
        // Add task, then set priority, then exit
        String input = "1\nPrioritize me\nMEDIUM\n\n\n7\n1\nCRITICAL\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Priority updated."),
                "Should confirm priority update");
    }

    @Test
    void setPriority_notFound() throws Exception {
        String input = "7\n999\nHIGH\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Task not found."),
                "Should report task not found");
    }

    // ---- Option 8: Remove task ----

    @Test
    void removeTask_success() throws Exception {
        // Add task, then remove it, then exit
        String input = "1\nRemove me\nMEDIUM\n\n\n8\n1\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Task 1 removed."),
                "Should confirm task removed");
    }

    @Test
    void removeTask_notFound() throws Exception {
        String input = "8\n999\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Task not found."),
                "Should report task not found");
    }

    // ---- Option 9: Summary ----

    @Test
    void showSummary_empty() throws Exception {
        String input = "9\n0\n";
        String output = runCli(input);

        // TaskSummary.toString() includes "Total: 0"
        assertTrue(output.contains("Total: 0"),
                "Summary should show zero totals when no tasks exist");
    }

    @Test
    void showSummary_afterAddingTasks() throws Exception {
        // Add a task, then summary, then exit
        String input = "1\nSummary task\nHIGH\n\n\n9\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Total: 1"),
                "Summary should show total of 1 after adding one task");
        assertTrue(output.contains("TODO: 1"),
                "Summary should show 1 TODO task");
    }

    // ---- Option 10: Show overdue ----

    @Test
    void showOverdue_noOverdueTasks() throws Exception {
        String input = "10\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("No overdue tasks."),
                "Should display 'No overdue tasks.' when none exist");
    }

    @Test
    void showOverdue_withOverdueTask() throws Exception {
        // Add a task with past due date, then show overdue, then exit
        String pastDate = LocalDate.now().minusDays(5).format(DateTimeFormatter.ISO_LOCAL_DATE);
        String input = "1\nOverdue task\nHIGH\n\n" + pastDate + "\n10\n0\n";
        String output = runCli(input);

        // The overdue task should appear and "No overdue tasks." should NOT
        assertFalse(output.contains("No overdue tasks."),
                "Should not say 'No overdue tasks.' when an overdue task exists");
        // The task title appears at least twice: once in "Created:" and once in overdue list
        long occurrences = output.split("Overdue task", -1).length - 1;
        assertTrue(occurrences >= 2,
                "Overdue task should appear in both creation confirmation and overdue list");
    }

    // ---- Full workflow: add, start, complete, list, summary ----

    @Test
    void fullWorkflow_addStartCompleteListSummary() throws Exception {
        // Add task -> start -> complete -> list (sort by priority) -> summary -> exit
        // Option 2 (list) has a sort sub-menu; "1" selects "Priority (highest first)"
        String input = "1\nWorkflow task\nHIGH\nTest desc\n\n3\n1\n4\n1\n2\n1\n9\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Created:"), "Task should be created");
        assertTrue(output.contains("Task 1 is now IN_PROGRESS."), "Task should be started");
        assertTrue(output.contains("Task 1 completed."), "Task should be completed");
        assertTrue(output.contains("Done: 1"), "Summary should show 1 done task");
        assertTrue(output.contains("Goodbye!"), "Should exit with Goodbye!");
    }

    // ---- Option 11: Filter tasks by status ----

    @Test
    void option11_showsFilterSubMenu() throws Exception {
        // Select filter (option 11), choose an invalid sub-option to trigger message, then exit
        String input = "11\n9\n0\n";
        String output = runCli(input);

        assertTrue(output.contains("Filter by:"),
                "Option 11 should show the filter sub-menu");
        assertTrue(output.contains("Choose filter:"),
                "Filter sub-menu should show 'Choose filter:' prompt");
        assertTrue(output.contains("Invalid filter option."),
                "Invalid sub-option should show 'Invalid filter option.'");
    }
}
