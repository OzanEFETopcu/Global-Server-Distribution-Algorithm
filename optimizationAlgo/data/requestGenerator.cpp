#include <iostream>
#include <chrono>
#include <iomanip>
#include <thread>
#include <future>
using namespace std;

void simulateDay()
{
    const int minutesInADay = 1440;
    const int scaleFactor = 200;
    const int simulationIntervalMs = 60 *1000 / scaleFactor; // Milliseconds for each simulated seconds ( 0.3 seconds = 1 minute in real life)

    //while (true)
    //{
        // Set initial simulation time to 00:00:00 (midnight)
        int simulatedMinutes = 0;

        while (simulatedMinutes < minutesInADay)
        {
            // Calculate hours, minutes, and seconds for simulated time
            int hours = (simulatedMinutes / 60) % 24;
            int minutes = simulatedMinutes % 60;

            // Print out simulated time
            std::cout << std::setfill('0') << std::setw(2) << hours << ":"
                      << std::setfill('0') << std::setw(2) << minutes << "\n";

            // Wait for the interval to pass (0.3s simulates 1 minute in scaled time)
            std::this_thread::sleep_for(std::chrono::milliseconds(simulationIntervalMs));

            // Increment simulated time
            simulatedMinutes++;
        }
    //}
};


void generateRequests(){}