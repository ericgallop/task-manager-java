package com.demo;

import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.File;
import java.time.LocalDate;

/**
 * Captures screenshots of the TaskManagerUI for each filter option.
 */
public class FilterScreenshotTest {

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

    public static void main(String[] args) throws Exception {
        String outputDir = args.length > 0 ? args[0] : "/workspace/project/screenshots";
        new File(outputDir).mkdirs();

        String[] filters = {"All tasks", "Todo", "In progress", "Done", "Cancelled", "Overdue"};
        String[] fileNames = {"filter_all.png", "filter_todo.png", "filter_in_progress.png", "filter_done.png", "filter_cancelled.png", "filter_overdue.png"};

        for (int i = 0; i < filters.length; i++) {
            captureFilteredScreenshot(createServiceWithTasks(), filters[i], outputDir + "/" + fileNames[i]);
        }

        System.out.println("All filter screenshots captured in " + outputDir);
        System.exit(0);
    }

    private static void captureFilteredScreenshot(TaskService service, String filterName, String filename) throws Exception {
        java.util.concurrent.CountDownLatch latch = new java.util.concurrent.CountDownLatch(1);

        SwingUtilities.invokeAndWait(() -> {
            try {
                UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
            } catch (Exception ignored) {}

            TaskManagerUI ui = new TaskManagerUI(service);
            ui.setSize(1100, 700);
            ui.setLocationRelativeTo(null);
            ui.setVisible(true);

            // Set the filter
            setFilter(ui, filterName);

            Timer timer = new Timer(500, e -> {
                try {
                    BufferedImage img = new BufferedImage(ui.getWidth(), ui.getHeight(), BufferedImage.TYPE_INT_RGB);
                    ui.paint(img.getGraphics());
                    ImageIO.write(img, "png", new File(filename));
                    System.out.println("Captured: " + filename + " [filter=" + filterName + "]");
                } catch (Exception ex) {
                    ex.printStackTrace();
                } finally {
                    ui.dispose();
                    latch.countDown();
                }
            });
            timer.setRepeats(false);
            timer.start();
        });

        latch.await(10, java.util.concurrent.TimeUnit.SECONDS);
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
}
