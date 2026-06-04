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
 * End-to-end tests for the CLI "Filter tasks by status" sub-menu (menu item 11).
 * Uses ProcessBuilder to launch Main with --cli and pipe stdin commands.
 */
public class CliFilterE2ETest {

    private static String classpath;

    @BeforeAll
    static void resolveClasspath() {
        // Use the Maven-compiled classes directory
        Path targetClasses = Paths.get("target", "classes").toAbsolutePath();
        classpath = targetClasses.toString();
    }

    /**
     * Helper: launches the CLI with the given stdin input and returns all stdout.
     */
    private String runCli(String stdinInput) throws Exception {
        ProcessBuilder pb = new ProcessBuilder(
                "java", "-cp", classpath, "com.demo.Main", "--cli"
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

    @Test
    void filterTodoAfterCreatingTask_showsTaskInOutput() throws Exception {
        // Create a task (option 1), then filter by Todo (option 11 -> 2), then exit (0)
        // Add task requires: title, priority, description, due date
        String input = "1\nBuy groceries\nHIGH\nWeekly shopping\n\n11\n2\n0\n";

        String output = runCli(input);

        // Verify the task was created
        assertTrue(output.contains("Created:"), "Should confirm task creation");
        assertTrue(output.contains("Buy groceries"), "Created task should include title");

        // Verify the filter sub-menu was displayed
        assertTrue(output.contains("Filter by:"), "Should show filter sub-menu header");
        assertTrue(output.contains("2. Todo"), "Should show Todo filter option");

        // After filter selection, the TODO task should appear in the filtered output
        // The task line appears twice: once after creation, once in filter results
        // Count occurrences of the task title to verify it appears in filter output too
        String afterFilter = output.substring(output.lastIndexOf("Choose filter:"));
        assertTrue(afterFilter.contains("Buy groceries"),
                "Filtered output should contain the TODO task");
        assertTrue(afterFilter.contains("[TODO]"),
                "Filtered output should show TODO status");
    }

    @Test
    void filterWithInvalidOption_showsInvalidFilterMessage() throws Exception {
        // Select filter menu (11), then enter invalid sub-option (9), then exit (0)
        String input = "11\n9\n0\n";

        String output = runCli(input);

        assertTrue(output.contains("Filter by:"), "Should show filter sub-menu");
        assertTrue(output.contains("Invalid filter option."),
                "Should display 'Invalid filter option.' for invalid sub-choice");
    }

    @Test
    void filterDoneWithNoMatchingTasks_showsNoTasksMessage() throws Exception {
        // Create a TODO task, then filter by Done (11 -> 4) — no done tasks exist
        String input = "1\nSome task\nMEDIUM\n\n\n11\n4\n0\n";

        String output = runCli(input);

        // The task is in TODO status, filtering by Done should show no matches
        assertTrue(output.contains("No tasks match the selected filter."),
                "Should display 'No tasks match the selected filter.' when no done tasks exist");
    }

    @Test
    void filterAllTasks_showsAllCreatedTasks() throws Exception {
        // Create two tasks, then filter by All (11 -> 1), then exit
        String input = "1\nTask Alpha\nHIGH\n\n\n1\nTask Beta\nLOW\n\n\n11\n1\n0\n";

        String output = runCli(input);

        String afterFilter = output.substring(output.lastIndexOf("Choose filter:"));
        assertTrue(afterFilter.contains("Task Alpha"),
                "All filter should show first task");
        assertTrue(afterFilter.contains("Task Beta"),
                "All filter should show second task");
    }

    @Test
    void filterEmptyRepository_showsNoTasksMessage() throws Exception {
        // Without creating any tasks, filter by Todo (11 -> 2)
        String input = "11\n2\n0\n";

        String output = runCli(input);

        assertTrue(output.contains("No tasks match the selected filter."),
                "Should display no-match message when repository is empty");
    }
}
