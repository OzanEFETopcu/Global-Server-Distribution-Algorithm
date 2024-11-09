#include <ostream>  // std::cout.
#include <vector>  // vectors.
#include <thread>  // threads.
#include "messageReceiver.h"
using namespace std;


int main()
{
    RegionalAlgo regionOregon("Oregon");
    RegionalAlgo regionLondon("London");
    RegionalAlgo regionSingapore("Singapore");

    // Create a vector to store the algorithms for scaling
    vector<RegionalAlgo> regions= {regionOregon, regionLondon, regionSingapore};
    // Create a vector to store the threads
    vector<thread> threads;


    // Launch a thread for each region
    /*for (auto& region : regions) {
        threads.emplace_back([&region]() { region.messageReceiver(); });  // Pass the region name to each thread
    }*/
    regions[0].messageReceiver();

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }

    return 0;

}