package com.demo;

import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.Timeout;

import java.io.*;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.concurrent.TimeUnit;

import static org.junit.jupiter.api.Assertions.*;

/**
 * E2E tests for the CLI filter-by-status sub-menu (menu item 11).
 * Each test launches a fresh process via ProcessBuilder so that
 * Main's static fields and System.in/out are isolated from JUnit.
 */
public class CliFilterE2ETest {

    private static String javaCmd;
    private static String classpath;

    @BeforeAll
    static void ensureCompiled() throws Exception {
        // Resolve paths
        javaCmd = ProcessHandle.current().info().command().orElse("java");
        Path projectDir = Paths.get(System.getProperty("user.dir"));
        classpath = projectDir.resolve("target/classes").toString();

        // Verify classes directory exists (mvn compile runs before test phase)
        File classesDir = new File(classpath);
        assertTrue(classesDir.isDirectory(),
                "target/classes must exist — run 'mvn compile' first");
        assertTrue(new File(classesDir, "com/demo/Main.class").exists(),
                "Main.class must exist in target/classes");
    }

    /**
     * Helper: launches the CLI with the given stdin lines and returns stdout.
     */
    private String runCli(String... inputLines) throws Exception {
        ProcessBuilder pb = new ProcessBuilder(
                javaCmd, "-cp", classpath, "com.demo.Main", "--cli");
        pb.redirectErrorStream(true);

        Process proc = pb.start();

        // Write all input lines, then close stdin
        try (OutputStream os = proc.getOutputStream()) {
            for (String line : inputLines) {
                os.write((line + "\n").getBytes());
            }
            os.flush();
        }

        // Read all output
        String output;
        try (BufferedReader reader = new BufferedReader(
                new InputStreamReader(proc.getInputStream()))) {
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                sb.append(line).append("\n");
            }
            output = sb.toString();
        }

        boolean finished = proc.waitFor(10, TimeUnit.SECONDS);
        if (!finished) {
            proc.destroyForcibly();
            fail("CLI process did not exit within 10 seconds");
        }

        return output;
    }

    @Test
    @Timeout(30)
    void filterTodo_showsCreatedTask() throws Exception {
        // Create a task (menu 1), then filter by Todo (menu 11 -> sub 2), then exit
        // Inputs for addTask: title, priority, description, due date
        String output = runCli(
                "1",                // Add task
                "Buy groceries",    // Title
                "HIGH",             // Priority
                "",                 // Description (empty)
                "",                 // Due date (empty)
                "11",               // Filter tasks by status
                "2",                // Sub-menu: Todo
                "0"                 // Exit
        );

        // The filter sub-menu should have been displayed
        assertTrue(output.contains("Filter by:"),
                "Expected filter sub-menu header in output");
        assertTrue(output.contains("All tasks"),
                "Expected 'All tasks' option in filter sub-menu");

        // After selecting Todo filter, our task should appear
        // The task output comes after "Choose filter:" prompt
        assertTrue(output.contains("[TODO]"),
                "Expected [TODO] status in filtered output");
        assertTrue(output.contains("Buy groceries"),
                "Expected task title 'Buy groceries' in filtered output");
        assertTrue(output.contains("[HIGH]"),
                "Expected [HIGH] priority in filtered output");
    }

    @Test
    @Timeout(30)
    void filterInvalidOption_showsErrorMessage() throws Exception {
        // Select filter menu (11), then invalid sub-option (9), then exit
        String output = runCli(
                "11",   // Filter tasks by status
                "9",    // Invalid filter option
                "0"     // Exit
        );

        assertTrue(output.contains("Invalid filter option."),
                "Expected 'Invalid filter option.' for invalid sub-menu choice");
    }

    @Test
    @Timeout(30)
    void filterDone_withNoCompletedTasks_showsNoMatch() throws Exception {
        // Create a TODO task, then filter by Done — should show no match
        String output = runCli(
                "1",                // Add task
                "Pending work",     // Title
                "",                 // Priority (default MEDIUM)
                "",                 // Description (empty)
                "",                 // Due date (empty)
                "11",               // Filter tasks by status
                "4",                // Sub-menu: Done
                "0"                 // Exit
        );

        assertTrue(output.contains("No tasks match the selected filter."),
                "Expected 'No tasks match the selected filter.' when filtering Done with no done tasks");
    }

    @Test
    @Timeout(30)
    void filterAll_showsAllTasksRegardlessOfStatus() throws Exception {
        // Create two tasks, start one (IN_PROGRESS), filter by All — both should show
        String output = runCli(
                "1",                // Add task 1
                "Task Alpha",       // Title
                "LOW",              // Priority
                "",                 // Description
                "",                 // Due date
                "1",                // Add task 2
                "Task Beta",        // Title
                "HIGH",             // Priority
                "",                 // Description
                "",                 // Due date
                "3",                // Start task (menu 3)
                "2",                // Task ID 2
                "11",               // Filter tasks by status
                "1",                // Sub-menu: All tasks
                "0"                 // Exit
        );

        // Both tasks should appear in output after the "All tasks" filter
        assertTrue(output.contains("Task Alpha"),
                "Expected 'Task Alpha' in All filter output");
        assertTrue(output.contains("Task Beta"),
                "Expected 'Task Beta' in All filter output");
    }

    @Test
    @Timeout(30)
    void filterCancelled_showsOnlyCancelledTasks() throws Exception {
        // Create two tasks, cancel one, filter by Cancelled — only cancelled should show
        String output = runCli(
                "1",                // Add task 1
                "Keep this",        // Title
                "",                 // Priority
                "",                 // Description
                "",                 // Due date
                "1",                // Add task 2
                "Cancel this",      // Title
                "",                 // Priority
                "",                 // Description
                "",                 // Due date
                "5",                // Cancel task (menu 5)
                "2",                // Task ID 2
                "11",               // Filter tasks by status
                "5",                // Sub-menu: Cancelled
                "0"                 // Exit
        );

        // After the filter, we need to check the output section after "Choose filter:"
        // The cancelled task should appear
        assertTrue(output.contains("[CANCELLED]"),
                "Expected [CANCELLED] status in filtered output");
        assertTrue(output.contains("Cancel this"),
                "Expected 'Cancel this' in cancelled filter output");
    }
}
