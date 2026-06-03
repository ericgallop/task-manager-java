#ifndef DATE_UTILS_H
#define DATE_UTILS_H

#include <string>
#include <ctime>

// Validates that a date string is in YYYY-MM-DD format with valid month/day ranges.
inline bool is_valid_date(const std::string& s) {
    if (s.size() != 10) return false;
    // Check format: DDDD-DD-DD (digits and dashes)
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) {
            if (s[i] != '-') return false;
        } else {
            if (s[i] < '0' || s[i] > '9') return false;
        }
    }
    int month = std::stoi(s.substr(5, 2));
    int day = std::stoi(s.substr(8, 2));
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;
    return true;
}

// Returns today's date as a YYYY-MM-DD string.
inline std::string today_as_string() {
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", tm);
    return std::string(buf);
}

#endif // DATE_UTILS_H
