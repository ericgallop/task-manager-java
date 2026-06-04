package com.demo;

import javax.swing.*;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.TableModel;
import java.awt.*;
import java.time.LocalDate;

/**
 * Programmatic validation of the visual rendering attributes in the TaskManagerUI.
 * Verifies colors for overdue, done, cancelled tasks match acceptance criteria.
 */
public class VisualValidationTest {

    public static void main(String[] args) throws Exception {
        int[] results = {0, 0}; // [pass, fail]

        SwingUtilities.invokeAndWait(() -> {
            try {
                UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
            } catch (Exception ignored) {}

            TaskService service = new TaskService(new InMemoryTaskRepository());

            // Create tasks with different statuses
            // Task 1: TODO, overdue (past due date, non-terminal)
            Task t1 = service.createTask("Overdue Task", Priority.HIGH, null, LocalDate.of(2024, 1, 15));
            service.assignTask(t1.getId(), "alice");

            // Task 2: IN_PROGRESS (not overdue)
            Task t2 = service.createTask("Active Task", Priority.CRITICAL, null, LocalDate.now().plusDays(30));
            service.assignTask(t2.getId(), "bob");
            service.startTask(t2.getId());

            // Task 3: DONE
            Task t3 = service.createTask("Done Task", Priority.MEDIUM, null, null);
            service.assignTask(t3.getId(), "charlie");
            service.completeTask(t3.getId());

            // Task 4: CANCELLED
            Task t4 = service.createTask("Cancelled Task", Priority.LOW, null, null);
            service.assignTask(t4.getId(), "diana");
            service.cancelTask(t4.getId());

            // Task 5: TODO, not overdue
            Task t5 = service.createTask("Normal Task", Priority.MEDIUM, null, LocalDate.now().plusDays(10));

            TaskManagerUI ui = new TaskManagerUI(service);
            ui.setSize(1100, 700);
            ui.setVisible(true);

            // Find the JTable
            JTable table = findTable(ui);
            if (table == null) {
                System.out.println("FAIL: Could not find JTable in UI");
                results[1]++;
                ui.dispose();
                return;
            }

            TableModel model = table.getModel();
            System.out.println("=== TABLE STRUCTURE ===");
            System.out.println("Columns: " + model.getColumnCount());
            for (int c = 0; c < model.getColumnCount(); c++) {
                System.out.println("  Column " + c + ": " + model.getColumnName(c));
            }
            System.out.println("Rows: " + model.getRowCount());
            System.out.println();

            // Validate AC1: All 6 columns present
            String[] expectedCols = {"ID", "Title", "Status", "Priority", "Due", "Assignee"};
            assertEqualsInt("Column count", 6, model.getColumnCount(), results);
            for (int i = 0; i < expectedCols.length; i++) {
                assertEquals("Column " + i + " name", expectedCols[i], model.getColumnName(i), results);
            }

            // Validate data is present
            assertEqualsInt("Row count", 5, model.getRowCount(), results);

            // Print all rows for evidence
            System.out.println("=== ROW DATA ===");
            for (int r = 0; r < model.getRowCount(); r++) {
                StringBuilder sb = new StringBuilder("Row " + r + ": ");
                for (int c = 0; c < model.getColumnCount(); c++) {
                    sb.append(model.getColumnName(c) + "=" + model.getValueAt(r, c) + " | ");
                }
                System.out.println(sb);
            }
            System.out.println();

            // Validate AC3: Visual distinction
            TableCellRenderer renderer = table.getDefaultRenderer(Object.class);
            System.out.println("=== VISUAL VALIDATION ===");

            // Check each row's rendering
            for (int r = 0; r < model.getRowCount(); r++) {
                Component comp = renderer.getTableCellRendererComponent(table, model.getValueAt(r, 1), false, false, r, 1);
                Color bg = comp.getBackground();
                Color fg = comp.getForeground();
                Object status = model.getValueAt(r, 2);
                Object title = model.getValueAt(r, 1);
                System.out.println("Row " + r + " [" + status + "] '" + title + "': bg=" + colorToString(bg) + " fg=" + colorToString(fg));
            }
            System.out.println();

            // Overdue task (row 0 after priority sort - should be CRITICAL first)
            // Find the overdue row
            int overdueRow = -1, doneRow = -1, cancelledRow = -1, normalRow = -1;
            for (int r = 0; r < model.getRowCount(); r++) {
                String title = model.getValueAt(r, 1).toString();
                if (title.equals("Overdue Task")) overdueRow = r;
                if (title.equals("Done Task")) doneRow = r;
                if (title.equals("Cancelled Task")) cancelledRow = r;
                if (title.equals("Normal Task")) normalRow = r;
            }

            // Validate overdue task has warm/orange background
            if (overdueRow >= 0) {
                Component comp = renderer.getTableCellRendererComponent(table, model.getValueAt(overdueRow, 1), false, false, overdueRow, 1);
                Color bg = comp.getBackground();
                assertEquals("Overdue bg", "rgb(255,244,230)", colorToString(bg), results);
                System.out.println("PASS: Overdue task has distinct warm/orange highlight background");
            } else {
                System.out.println("FAIL: Could not find overdue task row");
                results[1]++;
            }

            // Validate done task has green background
            if (doneRow >= 0) {
                Component comp = renderer.getTableCellRendererComponent(table, model.getValueAt(doneRow, 1), false, false, doneRow, 1);
                Color bg = comp.getBackground();
                assertEquals("Done bg", "rgb(236,248,241)", colorToString(bg), results);
                System.out.println("PASS: Done task has distinct green 'completed' background");
            } else {
                System.out.println("FAIL: Could not find done task row");
                results[1]++;
            }

            // Validate cancelled task has grey background AND dimmed foreground
            if (cancelledRow >= 0) {
                Component comp = renderer.getTableCellRendererComponent(table, model.getValueAt(cancelledRow, 1), false, false, cancelledRow, 1);
                Color bg = comp.getBackground();
                Color fg = comp.getForeground();
                assertEquals("Cancelled bg", "rgb(243,244,246)", colorToString(bg), results);
                assertEquals("Cancelled fg (dimmed)", "rgb(95,99,104)", colorToString(fg), results);
                System.out.println("PASS: Cancelled task has grey background AND dimmed foreground text");
            } else {
                System.out.println("FAIL: Could not find cancelled task row");
                results[1]++;
            }

            // Validate normal task has white background
            if (normalRow >= 0) {
                Component comp = renderer.getTableCellRendererComponent(table, model.getValueAt(normalRow, 1), false, false, normalRow, 1);
                Color bg = comp.getBackground();
                assertEquals("Normal bg", "rgb(255,255,255)", colorToString(bg), results);
                System.out.println("PASS: Normal task has white background (no special styling)");
            }

            ui.dispose();
        });

        System.out.println("\n=== RESULTS ===");
        System.out.println("Passed: " + results[0] + ", Failed: " + results[1]);
        System.exit(results[1] > 0 ? 1 : 0);
    }

    private static JTable findTable(Container container) {
        for (Component comp : container.getComponents()) {
            if (comp instanceof JTable) return (JTable) comp;
            if (comp instanceof JScrollPane) {
                JScrollPane sp = (JScrollPane) comp;
                Component view = sp.getViewport().getView();
                if (view instanceof JTable) return (JTable) view;
            }
            if (comp instanceof Container) {
                JTable found = findTable((Container) comp);
                if (found != null) return found;
            }
        }
        return null;
    }

    private static String colorToString(Color c) {
        return "rgb(" + c.getRed() + "," + c.getGreen() + "," + c.getBlue() + ")";
    }

    private static void assertEqualsInt(String label, int expected, int actual, int[] results) {
        if (expected == actual) {
            System.out.println("PASS: " + label + " = " + actual);
            results[0]++;
        } else {
            System.out.println("FAIL: " + label + " expected=" + expected + " actual=" + actual);
            results[1]++;
        }
    }

    private static void assertEquals(String label, String expected, String actual, int[] results) {
        if (expected.equals(actual)) {
            System.out.println("PASS: " + label + " = " + actual);
            results[0]++;
        } else {
            System.out.println("FAIL: " + label + " expected=" + expected + " actual=" + actual);
            results[1]++;
        }
    }
}
