// constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <map>
#include <string>
#include <optional>
#include <cmath> // For std::ceil

// Namespace to encapsulate the constants and functions, avoiding polluting the global namespace.
namespace Constants {

// Average cloud boot duration in seconds
inline constexpr double averageServerBootDuration = 11.381333;

// Memory and processor coefficients
inline constexpr double memoryCoefficient = 2.5; // GB
inline constexpr double logicalProcessorCoefficient = 1.4; // vCPU or Logical Processors
inline constexpr double memoryConstant = 3; // GB
inline constexpr double logicalProcessorConstant = 2.2; // vCPU or Logical Processors
inline constexpr double averageApplicationExecutionDuration = 60; // seconds. 200 minutes in real life

// Function declarations
inline int vCPUReqCalculator(int totalProcessNum) {
    double estimateRequirement = logicalProcessorCoefficient * (totalProcessNum - 1) + logicalProcessorConstant;
    return std::ceil(estimateRequirement);
}

inline int memoryReqCalculator(int totalProcessNum) {
    double estimateRequirement = memoryCoefficient * (totalProcessNum - 1) + memoryConstant;
    return std::ceil(estimateRequirement);
}

// Average regional service pricing in USD
inline const std::map<std::string, float> oregonServerPricing = {
    {"c08", 0.2859},
    {"c16", 0.5719},
    {"c32", 1.1437},
    {"c52", 1.8655},
    {"c88", 3.1869},
};

inline const std::map<std::string, float> londonServerPricing = {
    {"c08", 0.3547},
    {"c16", 0.7095},
    {"c32", 1.4190},
    {"c52", 2.3217},
    {"c88", 3.9969},
};

inline const std::map<std::string, float> singaporeServerPricing = {
    {"c08", 0.3412},
    {"c16", 0.6824},
    {"c32", 1.3648},
    {"c52", 2.2316},
    {"c88", 3.8360},
};

inline const std::map<std::string, std::map<std::string, float>>  overallServerPricing = {
    {"Oregon", oregonServerPricing},
    {"London", oregonServerPricing},
    {"Singapore", oregonServerPricing}
};

struct Capacity {
    int minThreshold;
    int maxThreshold;
    int absoluteLimit;
};

// Server process capacities per type
inline const std::map<std::string, Capacity> processCapacityPerInstanceType = {
    {"c08", {2, 3, 5}},
    {"c16", {3, 7, 10}},
    {"c32", {5, 19, 22}},
    {"c52", {15, 33, 36}},
    {"c88", {28, 58, 62}}
};

// Returns the next larger instance type, or nullopt if already at largest
std::optional<std::string> getNextInstanceType(const std::string& currentType) {
    // Find current type in map
    auto currentIt = processCapacityPerInstanceType.find(currentType);
    if (currentIt == processCapacityPerInstanceType.end()) {
        return std::nullopt;  // Current type not found
    }

    // Try to get next element
    auto nextIt = std::next(currentIt);
    if (nextIt == processCapacityPerInstanceType.end()) {
        return std::nullopt;  // Already at largest instance type
    }

    return nextIt->first;  // Return the next instance type
}

} // namespace Constants

#endif // CONSTANTS_H
