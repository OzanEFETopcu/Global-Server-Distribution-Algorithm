#include <iostream>
#include "requestGenerator.h"
#include <vector>
#include <thread>
using namespace std;

int main() {
    cout << "Starting 200x day simulation...\n";

    // Define the regions you want to simulate
    vector<string> regions = {"Oregon", "London", "Singapore"};
    // Create a vector to store the threads
    vector<thread> threads;

    // Launch a thread for each region
    for (const auto& region : regions) {
        threads.emplace_back(generateRequests, region);  // Pass the region name to each thread
    }


    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}