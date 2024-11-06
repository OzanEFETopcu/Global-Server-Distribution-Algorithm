#include <map>
#include <string>
using namespace std;

double averageCloudBootDuration = 11.381333; // seconds
double memoryCoefficient = 2.5; // GB
double logicalProcessorCoefficient = 1.4; // vCPU or Logical Processors
double memoryConstant = 3; // GB
double logicalProcessorConstant = 2.2; // vCPU or Logical Processors

int vCPUReqCalculator(int totalProcessNum){
    double estimateRequirement = logicalProcessorCoefficient * (totalProcessNum - 1) + logicalProcessorConstant;
    return ceil(estimateRequirement);
}

int memoryReqCalculator(int totalProcessNum){
    double estimateRequirement = memoryCoefficient * (totalProcessNum - 1) + memoryConstant;
    return ceil(estimateRequirement);
}

///////
// Average regional service pricing in USD
///////
map<string, float> oregonServerPricing = {
    {"c4", 0.1430},
    {"c8", 0.2859},
    {"c16", 0.5719},
    {"c32", 1.1437},
    {"c52", 1.8655},
    {"c88", 3.1869},
};

map<string, float> londonServerPricing = {
    {"c4", 0.1774},
    {"c8", 0.3547},
    {"c16", 0.7095},
    {"c32", 1.4190},
    {"c52", 2.3217},
    {"c88", 3.9969},
};

map<string, float> singaporeServerPricing = {
    {"c4", 0.1706},
    {"c8", 0.3412},
    {"c16", 0.6824},
    {"c32", 1.3648},
    {"c52", 2.2316},
    {"c88", 3.8360},
};


///////
// Average inter/intra-regional service latency in milliseconds
///////
map<string, float> oregonToDest = {
    {"oregon", 20},
    {"singapore", 380},
    {"london", 260},
};

map<string, float> singaporeToDest = {
    {"oregon", 370},
    {"singapore", 1},
    {"london", 400},
};

map<string, float> londonToDest = {
    {"oregon", 320},
    {"singapore", 320},
    {"london", 1},
};