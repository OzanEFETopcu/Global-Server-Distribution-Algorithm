#include <ostream>  // std::cout.
#include <vector>  // vectors.
#include <thread>  // threads.
#include "mqttSubscribeMessage.h"
#include "messageReceiver.h"
using namespace std;

RegionalAlgo::RegionalAlgo(string regionNameInput){
    regionName = regionNameInput;
}

void RegionalAlgo::messageReceiver(){

    auto client = initiateSubClient("subscribe_" + regionName, regionName);

    bool running = true;
    while (running)
    {
        // Construct a message pointer to hold an incoming message.
        mqtt::const_message_ptr messagePointer;

        // Try to consume a message, passing messagePointer by reference.
        // If a message is consumed, the function will return `true`,
        // allowing control to enter the if-statement body.
        if (client->try_consume_message(&messagePointer))
        {
            // Construct a string from the message payload.
            string messageString = messagePointer -> get_payload_str();
            // Print payload string to console (debugging).
            cout << messageString << endl;

            // Perform processing on the string.
            // This is where message processing can be passed onto different
            // functions for parsing.
            // Here, we break the loop and exit the program if a `quit` is received.
            if (messageString == "quit")
            {
                running = false;
            }
        }
    }
}