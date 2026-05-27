package com.demo;

public enum Priority {
    LOW, MEDIUM, HIGH, CRITICAL;

    public static Priority fromString(String s) {
        try {
            return valueOf(s.toUpperCase());
        } catch (IllegalArgumentException e) {
            return MEDIUM;
        }
    }
}
