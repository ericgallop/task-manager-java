package com.demo;

public enum TaskStatus {
    TODO, IN_PROGRESS, DONE, CANCELLED;

    public boolean isTerminal() {
        return this == DONE || this == CANCELLED;
    }
}
