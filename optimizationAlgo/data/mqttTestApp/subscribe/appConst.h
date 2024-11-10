// constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <map>
#include <string>
#include <cmath> // For std::ceil

// Namespace to encapsulate the constants and functions, avoiding polluting the global namespace.
namespace Constants {

// Average cloud boot duration in seconds
inline constexpr double averageCloudBootDuration = 11.381333;

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
    {"c8", 0.2859},
    {"c16", 0.5719},
    {"c32", 1.1437},
    {"c52", 1.8655},
    {"c88", 3.1869},
};

inline const std::map<std::string, float> londonServerPricing = {
    {"c8", 0.3547},
    {"c16", 0.7095},
    {"c32", 1.4190},
    {"c52", 2.3217},
    {"c88", 3.9969},
};

inline const std::map<std::string, float> singaporeServerPricing = {
    {"c8", 0.3412},
    {"c16", 0.6824},
    {"c32", 1.3648},
    {"c52", 2.2316},
    {"c88", 3.8360},
};

// Average inter/intra-regional service latency in milliseconds
inline const std::map<std::string, float> oregonToDest = {
    {"oregon", 20},
    {"singapore", 380},
    {"london", 260},
};

inline const std::map<std::string, float> singaporeToDest = {
    {"oregon", 370},
    {"singapore", 1},
    {"london", 400},
};

inline const std::map<std::string, float> londonToDest = {
    {"oregon", 320},
    {"singapore", 320},
    {"london", 1},
};



struct Capacity {
    int minThreshold;
    int maxThreshold;
    int absoluteLimit;
};

// Server process capacities per type
inline const std::map<std::string, Capacity> processCapacityPerInstanceType = {
    {"c8", {2, 3, 5}},
    {"c16", {3, 7, 10}},
    {"c32", {5, 19, 22}},
    {"c52", {15, 33, 36}},
    {"c88", {28, 58, 62}}
};

} // namespace Constants

#endif // CONSTANTS_H
