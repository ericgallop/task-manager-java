package com.demo;

import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.File;
import java.time.LocalDate;

/**
 * Captures screenshots of the TaskManagerUI for QA validation.
 * Run with: java -cp target/classes com.demo.ScreenshotTest
 */
public class ScreenshotTest {

    private static TaskService createServiceWithTasks() {
        TaskService service = new TaskService(new InMemoryTaskRepository());

        // Task 1: TODO, overdue, HIGH priority
        Task t1 = service.createTask("Fix login bug", Priority.HIGH, "Critical login issue", LocalDate.of(2024, 1, 15));
        service.assignTask(t1.getId(), "alice");

        // Task 2: IN_PROGRESS, CRITICAL priority
        Task t2 = service.createTask("Deploy to production", Priority.CRITICAL, "Release v2.0", LocalDate.now().plusDays(3));
        service.assignTask(t2.getId(), "bob");
        service.startTask(t2.getId());

        // Task 3: DONE, MEDIUM priority
        Task t3 = service.createTask("Write unit tests", Priority.MEDIUM, "Cover service layer", LocalDate.now().minusDays(2));
        service.assignTask(t3.getId(), "charlie");
        service.completeTask(t3.getId());

        // Task 4: CANCELLED, LOW priority
        Task t4 = service.createTask("Update docs", Priority.LOW, "Old feature docs", null);
        service.assignTask(t4.getId(), "diana");
        service.cancelTask(t4.getId());

        // Task 5: TODO, overdue, MEDIUM priority
        Task t5 = service.createTask("Review PR #42", Priority.MEDIUM, "Code review needed", LocalDate.of(2024, 6, 1));
        service.assignTask(t5.getId(), "eve");

        // Task 6: TODO, no due date, no assignee
        service.createTask("Brainstorm ideas", Priority.LOW, null, null);

        return service;
    }

    private static TaskService createEmptyService() {
        return new TaskService(new InMemoryTaskRepository());
    }

    public static void main(String[] args) throws Exception {
        String outputDir = args.length > 0 ? args[0] : "/workspace/project/screenshots";
        new File(outputDir).mkdirs();

        // Screenshot 1: Task list with all statuses (overdue, done, cancelled, in-progress, todo)
        captureScreenshot(createServiceWithTasks(), "All tasks", outputDir + "/all_tasks.png");

        // Screenshot 2: Empty task list
        captureScreenshot(createEmptyService(), "Empty", outputDir + "/empty_tasks.png");

        // Screenshot 3: Overdue filter
        captureFilteredScreenshot(createServiceWithTasks(), "Overdue", outputDir + "/overdue_filter.png");

        System.out.println("Screenshots captured successfully in " + outputDir);
        System.exit(0);
    }

    private static void captureScreenshot(TaskService service, String filterName, String filename) throws Exception {
        CountDownLatchWrapper latch = new CountDownLatchWrapper();

        SwingUtilities.invokeAndWait(() -> {
            try {
                UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
            } catch (Exception ignored) {}

            TaskManagerUI ui = new TaskManagerUI(service);
            ui.setSize(1100, 700);
            ui.setLocationRelativeTo(null);
            ui.setVisible(true);

            // Wait for rendering
            Timer timer = new Timer(500, e -> {
                try {
                    BufferedImage img = new BufferedImage(ui.getWidth(), ui.getHeight(), BufferedImage.TYPE_INT_RGB);
                    ui.paint(img.getGraphics());
                    ImageIO.write(img, "png", new File(filename));
                    System.out.println("Captured: " + filename);
                } catch (Exception ex) {
                    ex.printStackTrace();
                } finally {
                    ui.dispose();
                    latch.release();
                }
            });
            timer.setRepeats(false);
            timer.start();
        });

        latch.await();
    }

    private static void captureFilteredScreenshot(TaskService service, String filterName, String filename) throws Exception {
        CountDownLatchWrapper latch = new CountDownLatchWrapper();

        SwingUtilities.invokeAndWait(() -> {
            try {
                UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
            } catch (Exception ignored) {}

            TaskManagerUI ui = new TaskManagerUI(service);
            ui.setSize(1100, 700);
            ui.setLocationRelativeTo(null);
            ui.setVisible(true);

            // Find and set filter combo box
            setFilter(ui, filterName);

            Timer timer = new Timer(500, e -> {
                try {
                    BufferedImage img = new BufferedImage(ui.getWidth(), ui.getHeight(), BufferedImage.TYPE_INT_RGB);
                    ui.paint(img.getGraphics());
                    ImageIO.write(img, "png", new File(filename));
                    System.out.println("Captured: " + filename);
                } catch (Exception ex) {
                    ex.printStackTrace();
                } finally {
                    ui.dispose();
                    latch.release();
                }
            });
            timer.setRepeats(false);
            timer.start();
        });

        latch.await();
    }

    @SuppressWarnings("unchecked")
    private static void setFilter(Container container, String filterName) {
        for (Component comp : container.getComponents()) {
            if (comp instanceof JComboBox) {
                JComboBox<String> combo = (JComboBox<String>) comp;
                for (int i = 0; i < combo.getItemCount(); i++) {
                    if (filterName.equals(combo.getItemAt(i))) {
                        combo.setSelectedItem(filterName);
                        return;
                    }
                }
            }
            if (comp instanceof Container) {
                setFilter((Container) comp, filterName);
            }
        }
    }

    // Simple latch to coordinate EDT and main thread
    static class CountDownLatchWrapper {
        private final java.util.concurrent.CountDownLatch latch = new java.util.concurrent.CountDownLatch(1);
        void release() { latch.countDown(); }
        void await() throws InterruptedException { latch.await(10, java.util.concurrent.TimeUnit.SECONDS); }
    }
}
