package com.demo;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.time.LocalDate;
import java.time.format.DateTimeParseException;
import java.util.List;
import java.util.Locale;
import java.util.stream.Collectors;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.ListSelectionModel;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.border.EmptyBorder;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;

public class TaskManagerUI extends JFrame {
    private final TaskService service;
    private final TaskTableModel tableModel;

    private final JTextField titleField = new JTextField();
    private final JTextArea descriptionArea = new JTextArea(4, 24);
    private final JComboBox<Priority> priorityBox = new JComboBox<>(Priority.values());
    private final JTextField dueDateField = new JTextField();
    private final JTextField assigneeField = new JTextField();
    private final JComboBox<String> filterBox = new JComboBox<>(
            new String[] {"All tasks", "Todo", "In progress", "Done", "Cancelled", "Overdue"});
    private final JComboBox<String> sortBox = new JComboBox<>(
            new String[] {"Priority", "Due date", "Created"});
    private final JLabel summaryLabel = new JLabel();
    private JTable taskTable;

    public TaskManagerUI(TaskService service) {
        super("Task Manager");
        this.service = service;
        this.tableModel = new TaskTableModel();

        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setMinimumSize(new Dimension(980, 640));
        setLocationByPlatform(true);

        setContentPane(buildContent());
        refreshTasks();
    }

    public static void launch(TaskService service) {
        SwingUtilities.invokeLater(() -> {
            applyLookAndFeel();
            TaskManagerUI ui = new TaskManagerUI(service);
            ui.setVisible(true);
        });
    }

    private static void applyLookAndFeel() {
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception ignored) {
            // Swing's cross-platform look and feel is a fine fallback.
        }
    }

    private JPanel buildContent() {
        JPanel root = new JPanel(new BorderLayout(16, 16));
        root.setBorder(new EmptyBorder(18, 18, 18, 18));
        root.setBackground(new Color(246, 247, 249));

        root.add(buildHeader(), BorderLayout.NORTH);
        root.add(buildMainArea(), BorderLayout.CENTER);
        root.add(buildFooter(), BorderLayout.SOUTH);
        return root;
    }

    private JPanel buildHeader() {
        JPanel header = new JPanel(new BorderLayout(16, 8));
        header.setOpaque(false);

        JLabel title = new JLabel("Task Manager");
        title.setFont(title.getFont().deriveFont(Font.BOLD, 28f));

        JPanel controls = new JPanel(new FlowLayout(FlowLayout.RIGHT, 8, 0));
        controls.setOpaque(false);
        controls.add(new JLabel("Filter"));
        controls.add(filterBox);
        controls.add(new JLabel("Sort"));
        controls.add(sortBox);

        filterBox.addActionListener(e -> refreshTasks());
        sortBox.addActionListener(e -> refreshTasks());

        header.add(title, BorderLayout.WEST);
        header.add(controls, BorderLayout.EAST);
        return header;
    }

    private JPanel buildMainArea() {
        JPanel main = new JPanel(new BorderLayout(16, 0));
        main.setOpaque(false);
        main.add(buildFormPanel(), BorderLayout.WEST);
        main.add(buildTablePanel(), BorderLayout.CENTER);
        return main;
    }

    private JPanel buildFormPanel() {
        JPanel panel = new JPanel(new BorderLayout(0, 14));
        panel.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createLineBorder(new Color(218, 222, 229)),
                new EmptyBorder(16, 16, 16, 16)));
        panel.setBackground(Color.WHITE);
        panel.setPreferredSize(new Dimension(320, 100));

        JLabel heading = new JLabel("New task");
        heading.setFont(heading.getFont().deriveFont(Font.BOLD, 18f));

        JPanel fields = new JPanel(new GridBagLayout());
        fields.setOpaque(false);

        descriptionArea.setLineWrap(true);
        descriptionArea.setWrapStyleWord(true);
        priorityBox.setSelectedItem(Priority.MEDIUM);

        addFormRow(fields, 0, "Title", titleField);
        addFormRow(fields, 1, "Priority", priorityBox);
        addFormRow(fields, 2, "Due date", dueDateField);
        addFormRow(fields, 3, "Assignee", assigneeField);
        addFormRow(fields, 4, "Description", new JScrollPane(descriptionArea));

        dueDateField.setToolTipText("Use yyyy-MM-dd, or leave empty.");

        JButton addButton = new JButton("Add task");
        addButton.addActionListener(e -> addTask());

        JButton clearButton = new JButton("Clear");
        clearButton.addActionListener(e -> clearForm());

        JPanel actions = new JPanel(new FlowLayout(FlowLayout.LEFT, 8, 0));
        actions.setOpaque(false);
        actions.add(addButton);
        actions.add(clearButton);

        panel.add(heading, BorderLayout.NORTH);
        panel.add(fields, BorderLayout.CENTER);
        panel.add(actions, BorderLayout.SOUTH);
        return panel;
    }

    private void addFormRow(JPanel panel, int row, String labelText, Component field) {
        GridBagConstraints label = new GridBagConstraints();
        label.gridx = 0;
        label.gridy = row;
        label.anchor = GridBagConstraints.NORTHWEST;
        label.insets = new Insets(0, 0, 12, 10);

        JLabel jLabel = new JLabel(labelText);
        jLabel.setFont(jLabel.getFont().deriveFont(Font.BOLD));
        panel.add(jLabel, label);

        GridBagConstraints input = new GridBagConstraints();
        input.gridx = 1;
        input.gridy = row;
        input.weightx = 1;
        input.fill = GridBagConstraints.HORIZONTAL;
        input.insets = new Insets(0, 0, 12, 0);
        panel.add(field, input);
    }

    private JPanel buildTablePanel() {
        JPanel panel = new JPanel(new BorderLayout(0, 12));
        panel.setOpaque(false);

        taskTable = new JTable(tableModel);
        taskTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        taskTable.setRowHeight(30);
        taskTable.setShowGrid(false);
        taskTable.setIntercellSpacing(new Dimension(0, 0));
        taskTable.getTableHeader().setReorderingAllowed(false);
        taskTable.setDefaultRenderer(Object.class, new TaskCellRenderer());

        taskTable.getColumnModel().getColumn(0).setPreferredWidth(48);
        taskTable.getColumnModel().getColumn(1).setPreferredWidth(210);
        taskTable.getColumnModel().getColumn(2).setPreferredWidth(110);
        taskTable.getColumnModel().getColumn(3).setPreferredWidth(98);
        taskTable.getColumnModel().getColumn(4).setPreferredWidth(110);
        taskTable.getColumnModel().getColumn(5).setPreferredWidth(120);

        JScrollPane scrollPane = new JScrollPane(taskTable);
        scrollPane.setBorder(BorderFactory.createLineBorder(new Color(218, 222, 229)));
        scrollPane.getViewport().setBackground(Color.WHITE);

        panel.add(buildActionBar(), BorderLayout.NORTH);
        panel.add(scrollPane, BorderLayout.CENTER);
        return panel;
    }

    private JPanel buildActionBar() {
        JPanel bar = new JPanel(new FlowLayout(FlowLayout.LEFT, 8, 0));
        bar.setOpaque(false);

        JButton startButton = new JButton("Start");
        startButton.addActionListener(e -> withSelectedTask(task -> service.startTask(task.getId())));

        JButton doneButton = new JButton("Complete");
        doneButton.addActionListener(e -> withSelectedTask(task -> service.completeTask(task.getId())));

        JButton cancelButton = new JButton("Cancel");
        cancelButton.addActionListener(e -> withSelectedTask(task -> service.cancelTask(task.getId())));

        JButton deleteButton = new JButton("Delete");
        deleteButton.addActionListener(e -> deleteSelectedTask());

        JButton assignButton = new JButton("Assign");
        assignButton.addActionListener(e -> assignSelectedTask());

        JButton priorityButton = new JButton("Priority");
        priorityButton.addActionListener(e -> updateSelectedPriority());

        bar.add(startButton);
        bar.add(doneButton);
        bar.add(cancelButton);
        bar.add(deleteButton);
        bar.add(Box.createHorizontalStrut(16));
        bar.add(assignButton);
        bar.add(priorityButton);
        return bar;
    }

    private JPanel buildFooter() {
        JPanel footer = new JPanel();
        footer.setLayout(new BoxLayout(footer, BoxLayout.X_AXIS));
        footer.setOpaque(false);

        summaryLabel.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createLineBorder(new Color(218, 222, 229)),
                new EmptyBorder(10, 12, 10, 12)));
        summaryLabel.setOpaque(true);
        summaryLabel.setBackground(Color.WHITE);
        summaryLabel.setHorizontalAlignment(SwingConstants.LEFT);

        footer.add(summaryLabel);
        footer.add(Box.createHorizontalGlue());
        return footer;
    }

    private void addTask() {
        String title = titleField.getText().trim();
        if (title.isEmpty()) {
            showMessage("Title is required.");
            titleField.requestFocus();
            return;
        }

        LocalDate dueDate = parseDueDate();
        if (dueDateField.getText().trim().length() > 0 && dueDate == null) {
            showMessage("Due date must use yyyy-MM-dd.");
            dueDateField.requestFocus();
            return;
        }

        Task task = service.createTask(
                title,
                (Priority) priorityBox.getSelectedItem(),
                descriptionArea.getText().trim(),
                dueDate);

        String assignee = assigneeField.getText().trim();
        if (!assignee.isEmpty()) {
            service.assignTask(task.getId(), assignee);
        }

        clearForm();
        refreshTasks();
        selectTask(task.getId());
    }

    private LocalDate parseDueDate() {
        String value = dueDateField.getText().trim();
        if (value.isEmpty()) {
            return null;
        }

        try {
            return LocalDate.parse(value);
        } catch (DateTimeParseException e) {
            return null;
        }
    }

    private void clearForm() {
        titleField.setText("");
        descriptionArea.setText("");
        priorityBox.setSelectedItem(Priority.MEDIUM);
        dueDateField.setText("");
        assigneeField.setText("");
        titleField.requestFocus();
    }

    private void assignSelectedTask() {
        Task task = getSelectedTask();
        if (task == null) {
            showMessage("Select a task first.");
            return;
        }

        String current = task.getAssignee() == null ? "" : task.getAssignee();
        String assignee = JOptionPane.showInputDialog(this, "Assignee", current);
        if (assignee == null) {
            return;
        }

        service.assignTask(task.getId(), assignee.trim().isEmpty() ? null : assignee.trim());
        refreshTasks();
        selectTask(task.getId());
    }

    private void updateSelectedPriority() {
        Task task = getSelectedTask();
        if (task == null) {
            showMessage("Select a task first.");
            return;
        }

        JComboBox<Priority> priorities = new JComboBox<>(new DefaultComboBoxModel<>(Priority.values()));
        priorities.setSelectedItem(task.getPriority());
        int choice = JOptionPane.showConfirmDialog(
                this,
                priorities,
                "Set priority",
                JOptionPane.OK_CANCEL_OPTION,
                JOptionPane.PLAIN_MESSAGE);

        if (choice == JOptionPane.OK_OPTION) {
            service.updatePriority(task.getId(), (Priority) priorities.getSelectedItem());
            refreshTasks();
            selectTask(task.getId());
        }
    }

    private void deleteSelectedTask() {
        Task task = getSelectedTask();
        if (task == null) {
            showMessage("Select a task first.");
            return;
        }

        int choice = JOptionPane.showConfirmDialog(
                this,
                "Delete task #" + task.getId() + "?",
                "Delete task",
                JOptionPane.YES_NO_OPTION,
                JOptionPane.WARNING_MESSAGE);
        if (choice == JOptionPane.YES_OPTION) {
            service.deleteTask(task.getId());
            refreshTasks();
        }
    }

    private void withSelectedTask(TaskAction action) {
        Task task = getSelectedTask();
        if (task == null) {
            showMessage("Select a task first.");
            return;
        }

        try {
            boolean changed = action.apply(task);
            if (!changed) {
                showMessage("The selected task could not be updated.");
            }
        } catch (IllegalStateException e) {
            showMessage(e.getMessage());
        }

        refreshTasks();
        selectTask(task.getId());
    }

    private Task getSelectedTask() {
        int row = taskTable.getSelectedRow();
        if (row < 0) {
            return null;
        }
        return tableModel.getTask(row);
    }

    private void selectTask(int taskId) {
        for (int row = 0; row < tableModel.getRowCount(); row++) {
            if (tableModel.getTask(row).getId() == taskId) {
                taskTable.setRowSelectionInterval(row, row);
                taskTable.scrollRectToVisible(taskTable.getCellRect(row, 0, true));
                return;
            }
        }
    }

    private void refreshTasks() {
        tableModel.setTasks(getVisibleTasks());
        updateSummary();
    }

    private List<Task> getVisibleTasks() {
        List<Task> tasks;
        String sort = (String) sortBox.getSelectedItem();
        if ("Due date".equals(sort)) {
            tasks = service.getTasksSortedByDueDate();
        } else if ("Created".equals(sort)) {
            tasks = service.getTasksSortedByCreationOrder();
        } else {
            tasks = service.getTasksSortedByPriority();
        }

        String filter = (String) filterBox.getSelectedItem();
        return tasks.stream()
                .filter(task -> matchesFilter(task, filter))
                .collect(Collectors.toList());
    }

    private boolean matchesFilter(Task task, String filter) {
        if ("Todo".equals(filter)) {
            return task.getStatus() == TaskStatus.TODO;
        }
        if ("In progress".equals(filter)) {
            return task.getStatus() == TaskStatus.IN_PROGRESS;
        }
        if ("Done".equals(filter)) {
            return task.getStatus() == TaskStatus.DONE;
        }
        if ("Cancelled".equals(filter)) {
            return task.getStatus() == TaskStatus.CANCELLED;
        }
        if ("Overdue".equals(filter)) {
            return task.isOverdue();
        }
        return true;
    }

    private void updateSummary() {
        TaskSummary summary = service.getSummary();
        summaryLabel.setText(String.format(Locale.US,
                "Total %d    Todo %d    In progress %d    Done %d    Cancelled %d    Overdue %d",
                summary.total,
                summary.todo,
                summary.inProgress,
                summary.done,
                summary.cancelled,
                summary.overdue));
    }

    private void showMessage(String message) {
        JOptionPane.showMessageDialog(this, message, "Task Manager", JOptionPane.INFORMATION_MESSAGE);
    }

    private interface TaskAction {
        boolean apply(Task task);
    }

    private static class TaskTableModel extends AbstractTableModel {
        private static final String[] COLUMNS = {"ID", "Title", "Status", "Priority", "Due", "Assignee"};
        private List<Task> tasks = List.of();

        @Override
        public int getRowCount() {
            return tasks.size();
        }

        @Override
        public int getColumnCount() {
            return COLUMNS.length;
        }

        @Override
        public String getColumnName(int column) {
            return COLUMNS[column];
        }

        @Override
        public Object getValueAt(int rowIndex, int columnIndex) {
            Task task = tasks.get(rowIndex);
            switch (columnIndex) {
                case 0:
                    return task.getId();
                case 1:
                    return task.getTitle();
                case 2:
                    return task.getStatus();
                case 3:
                    return task.getPriority();
                case 4:
                    return task.getDueDate() == null ? "" : task.getDueDate();
                case 5:
                    return task.getAssignee() == null ? "" : task.getAssignee();
                default:
                    return "";
            }
        }

        public Task getTask(int row) {
            return tasks.get(row);
        }

        public void setTasks(List<Task> tasks) {
            this.tasks = tasks;
            fireTableDataChanged();
        }
    }

    private static class TaskCellRenderer extends DefaultTableCellRenderer {
        private static final Color OVERDUE = new Color(255, 244, 230);
        private static final Color DONE = new Color(236, 248, 241);
        private static final Color CANCELLED = new Color(243, 244, 246);
        private static final Color SELECTED = new Color(43, 108, 176);

        @Override
        public Component getTableCellRendererComponent(
                JTable table,
                Object value,
                boolean isSelected,
                boolean hasFocus,
                int row,
                int column) {
            Component component = super.getTableCellRendererComponent(
                    table, value, isSelected, hasFocus, row, column);

            TaskTableModel model = (TaskTableModel) table.getModel();
            Task task = model.getTask(row);

            if (isSelected) {
                component.setBackground(SELECTED);
                component.setForeground(Color.WHITE);
            } else if (task.isOverdue()) {
                component.setBackground(OVERDUE);
                component.setForeground(new Color(32, 33, 36));
            } else if (task.getStatus() == TaskStatus.DONE) {
                component.setBackground(DONE);
                component.setForeground(new Color(32, 33, 36));
            } else if (task.getStatus() == TaskStatus.CANCELLED) {
                component.setBackground(CANCELLED);
                component.setForeground(new Color(95, 99, 104));
            } else {
                component.setBackground(Color.WHITE);
                component.setForeground(new Color(32, 33, 36));
            }

            setBorder(new EmptyBorder(0, 8, 0, 8));
            return component;
        }
    }
}
