package com.demo;

import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

import static org.junit.jupiter.api.Assertions.*;

/**
 * End-to-end tests for the CLI "Show overdue" menu option (menu item 10).
 * Validates AC1 (overdue tasks visually highlighted), AC2 (terminal tasks excluded),
 * AC3 (no due date never overdue), and AC4 (dedicated overdue view shows only overdue).
 *
 * Uses ProcessBuilder to launch Main and pipe stdin commands.
 */
public class CliOverdueE2ETest {

    private static String classpath;

    @BeforeAll
    static void resolveClasspath() {
        Path targetClasses = Paths.get("target", "classes").toAbsolutePath();
        classpath = targetClasses.toString();
    }

    /**
     * Helper: launches the CLI with the given stdin input and returns all stdout.
     */
    private String runCli(String stdinInput) throws Exception {
        ProcessBuilder pb = new ProcessBuilder(
                "java", "-cp", classpath, "com.demo.Main"
        );
        pb.redirectErrorStream(true);
        pb.directory(Paths.get(".").toAbsolutePath().toFile());

        Process process = pb.start();

        try (OutputStream os = process.getOutputStream()) {
            os.write(stdinInput.getBytes(StandardCharsets.UTF_8));
            os.flush();
        }

        String output;
        try (BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8))) {
            output = reader.lines().collect(Collectors.joining("\n"));
        }

        boolean finished = process.waitFor(10, TimeUnit.SECONDS);
        assertTrue(finished, "CLI process did not terminate in time");
        return output;
    }

    @Test
    void showOverdue_withOverdueTask_displaysTaskWithOverdueMarker() throws Exception {
        // AC1 + AC4: Create a task with a past due date, then show overdue (option 10).
        // The overdue task should appear with the visual marker.
        // Input sequence:
        //   1           -> Add task
        //   Overdue bug -> Title
        //   HIGH        -> Priority
        //                -> Description (empty)
        //   2020-01-15  -> Due date (far in the past)
        //   10          -> Show overdue
        //   0           -> Exit
        String input = "1\nOverdue bug\nHIGH\n\n2020-01-15\n10\n0\n";

        String output = runCli(input);

        // Verify task was created with the overdue marker
        assertTrue(output.contains("Created:"), "Should confirm task creation");
        assertTrue(output.contains("Overdue bug"), "Output should contain the task title");
        assertTrue(output.contains("OVERDUE"),
                "Created task output should include OVERDUE marker");

        // After selecting option 10, the overdue task should be listed
        // Find the output after the second "Choose:" (which is for option 10)
        String afterShowOverdue = getOutputAfterNthChoose(output, 2);
        assertTrue(afterShowOverdue.contains("Overdue bug"),
                "Show overdue should display the overdue task");
        assertTrue(afterShowOverdue.contains("OVERDUE"),
                "Show overdue should display the task with OVERDUE marker");
        assertTrue(afterShowOverdue.contains("[TODO]"),
                "Show overdue should display the task status");
        assertTrue(afterShowOverdue.contains("2020-01-15"),
                "Show overdue should display the due date");

        // Should NOT show "No overdue tasks." message
        assertFalse(afterShowOverdue.startsWith("No overdue tasks."),
                "Should not show 'No overdue tasks.' when overdue tasks exist");
    }

    @Test
    void showOverdue_noOverdueTasks_displaysNoOverdueMessage() throws Exception {
        // AC3 + AC4: Create tasks with future due dates and no due dates,
        // then show overdue. Should display "No overdue tasks."
        // Input sequence:
        //   1             -> Add task
        //   Future task   -> Title
        //   MEDIUM        -> Priority
        //                  -> Description (empty)
        //   2030-12-31    -> Due date (far in the future)
        //   1             -> Add another task
        //   No due task   -> Title
        //   LOW           -> Priority
        //                  -> Description (empty)
        //                  -> Due date (empty = no due date)
        //   10            -> Show overdue
        //   0             -> Exit
        String input = "1\nFuture task\nMEDIUM\n\n2030-12-31\n1\nNo due task\nLOW\n\n\n10\n0\n";

        String output = runCli(input);

        // Verify both tasks were created
        assertTrue(output.contains("Future task"), "First task should be created");
        assertTrue(output.contains("No due task"), "Second task should be created");

        // After selecting option 10, should show "No overdue tasks." message
        String afterShowOverdue = getOutputAfterNthChoose(output, 3);
        assertTrue(afterShowOverdue.contains("No overdue tasks."),
                "Should display 'No overdue tasks.' when no tasks are overdue");
    }

    @Test
    void showOverdue_completedOverdueTask_excludedFromOverdueList() throws Exception {
        // AC2 + AC4: Create a task with a past due date, complete it, then show overdue.
        // The completed task should NOT appear in the overdue list.
        // Input sequence:
        //   1             -> Add task
        //   Late report   -> Title
        //   HIGH          -> Priority
        //                  -> Description (empty)
        //   2020-06-01    -> Due date (past)
        //   4             -> Complete task
        //   1             -> Task ID
        //   10            -> Show overdue
        //   0             -> Exit
        String input = "1\nLate report\nHIGH\n\n2020-06-01\n4\n1\n10\n0\n";

        String output = runCli(input);

        // Verify task was created
        assertTrue(output.contains("Created:"), "Should confirm task creation");
        assertTrue(output.contains("Late report"), "Output should contain the task title");

        // Verify task was completed
        assertTrue(output.contains("Task 1 completed."),
                "Should confirm task completion");

        // After selecting option 10, should show "No overdue tasks." since
        // the only task with a past due date has been completed (terminal status)
        String afterShowOverdue = getOutputAfterNthChoose(output, 3);
        assertTrue(afterShowOverdue.contains("No overdue tasks."),
                "Should display 'No overdue tasks.' after completing the overdue task");
        assertFalse(afterShowOverdue.contains("Late report"),
                "Completed task should not appear in overdue list");
    }

    /**
     * Returns the substring of output after the Nth occurrence of "Choose: ".
     * This helps isolate the output of a specific menu action.
     */
    private String getOutputAfterNthChoose(String output, int n) {
        String marker = "Choose: ";
        int index = 0;
        for (int i = 0; i < n; i++) {
            index = output.indexOf(marker, index);
            if (index < 0) {
                fail("Could not find occurrence #" + (i + 1) + " of '" + marker + "' in output:\n" + output);
            }
            index += marker.length();
        }
        return output.substring(index);
    }
}
