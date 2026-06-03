#ifndef PRIORITY_H
#define PRIORITY_H

#include <string>
#include <algorithm>

enum class Priority { LOW, MEDIUM, HIGH, CRITICAL };

inline Priority priority_from_string(const std::string& s) {
    std::string upper = s;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    if (upper == "LOW") return Priority::LOW;
    if (upper == "HIGH") return Priority::HIGH;
    if (upper == "CRITICAL") return Priority::CRITICAL;
    if (upper == "MEDIUM") return Priority::MEDIUM;
    return Priority::MEDIUM; // default for unrecognized/empty input
}

inline std::string priority_to_string(Priority p) {
    switch (p) {
        case Priority::LOW:      return "LOW";
        case Priority::MEDIUM:   return "MEDIUM";
        case Priority::HIGH:     return "HIGH";
        case Priority::CRITICAL: return "CRITICAL";
    }
    return "MEDIUM";
}

#endif // PRIORITY_H
