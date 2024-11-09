#include <iostream>
#include <random>
#include <mqtt/client.h>
#include "mqttPublishMessage.h"
#include <chrono>
#include <thread>
using namespace std;

void generateRequests(string regionName)
{
    // Set up random number generation
    random_device rd;
    mt19937 gen(rd());

    // Define high and low traffic frequency ranges for random durations
    uniform_int_distribution<> lowTrafficDis(10000, 20000); // In real life between 33 minute to 66 minutes a request
    uniform_int_distribution<> highTrafficDis(1000, 2000);  // In real life between 3.3 minute to 6.6 minutes a request

    auto client = initiatePubClient("publish_" + regionName);
    // Initialize an empty message with specified topic.
    mqtt::message_ptr timeLeftMessagePointer = mqtt::make_message(regionName, "");

    // Start timer to track how long the program has been running
    auto start = chrono::steady_clock::now();

    while (true)
    {
        // Calculate elapsed time
        auto now = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();

        // Choose distribution based on elapsed time
        int randomPause;
        if (elapsed < 144)
        {
            randomPause = lowTrafficDis(gen);
        }
        else if (elapsed < 288)
        {
            randomPause = highTrafficDis(gen);
        }
        else if (elapsed <= 432)
        {
            randomPause = lowTrafficDis(gen);
        }
        else
        {
            start = chrono::steady_clock::now();
            randomPause = lowTrafficDis(gen);
        }

        string payload = "Timestamp:" + to_string(elapsed) + "\tRegion: " + regionName + "=" + to_string(randomPause);
        publishMessage(payload, *client, timeLeftMessagePointer);

        // Pause the program for the random duration
        this_thread::sleep_for(chrono::milliseconds(randomPause));
    }
}