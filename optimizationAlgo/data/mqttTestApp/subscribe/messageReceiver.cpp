#include <ostream> // std::cout.
#include <vector>  // vectors.
#include <thread>  // threads.
#include <functional>
#include <future>
#include <mutex>
#include "mqttSubscribeMessage.h"
#include "messageReceiver.h"
#include "appConst.h"
using namespace std;
using namespace Constants;

//////////////////
// Regional algorithm class implementation
RegionalAlgo::RegionalAlgo(string regionNameInput)
{
    regionName = regionNameInput;
};

// Continuously listening to requests coming from outside and handling the requests
void RegionalAlgo::messageReceiver()
{

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
            string messageString = messagePointer->get_payload_str();
            // Print payload string to console (debugging).
            std::future<void> ft = std::async(std::launch::async, [this]()
                                              { addProcessToServer(); });

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

// As the requests come in adding the processes to servers
void RegionalAlgo::addProcessToServer()
{
    // First, get the appropriate server under a smaller lock scope
    std::shared_ptr<Server> targetServer;
    {
        std::lock_guard<std::mutex> lock(serversMutex);

        if (serverType0.size() > 0)
        {
            targetServer = serverType0.front();
        }
        else if (serverType1.size() > 0)
        {
            targetServer = serverType1.front();
        }
        else if (serverType2.size() > 0)
        {
            targetServer = serverType2.front();
        }
        else
        {
            // Need to add a new server
            auto newServer = std::make_shared<Server>("c4");
            serverType1.push_back(newServer);
            targetServer = newServer;
        }
    } // serversMutex is released here

    // Now launch the process on the selected server
    if (targetServer)
    {
        targetServer->launchProcess(Constants::averageApplicationExecutionDuration);
        cout << "Process launched successfully!\n\n";
        cout << "Infrastructure update:\n";
        cout << "---------------------------\n";
        cout << "Server Type 0 Amount: " << serverType0.size() << endl;
        cout << "Server Type 1 Amount: " << serverType1.size() << endl;
        cout << "Server Type 2 Amount: " << serverType2.size() << endl;
        cout << "Server Type 3 Amount: " << serverType3.size() << endl;
        cout << "---------------------------\n";
        cout << "Server Type 1 Process Num: " << serverType1.front()->getTotalProcessNum() << endl;
        cout << "---------------------------\n\n\n\n";
    }
}

// Adding a new server to the server pool of serverType1 since there is no processes in that server
void RegionalAlgo::addServer(string instanceTypeInput)
{
    std::lock_guard<std::mutex> lock(serversMutex);
    auto server = std::make_shared<Server>(instanceTypeInput);
    serverType1.push_back(server);
};

// Removing servers that are no more used
void RegionalAlgo::removeServer()
{
    std::lock_guard<std::mutex> lock(serversMutex);
    // Remove servers with no active processes from the serverType1 vector
    serverType1.erase(
        std::remove_if(serverType1.begin(), serverType1.end(),
                       [](const std::shared_ptr<Server> &server)
                       {
                           return server->getTotalProcessNum() == 0;
                       }),
        serverType1.end());
};

//////////////////
// Process class implementation
Process::Process(int executionTimeInput, function<void(std::shared_ptr<Process>)> onComplete)
{
    executionTime = executionTimeInput;
    onCompleteCallback = onComplete;
    start();
};

Process::~Process()
{
    if (processThread.joinable())
    {
        if (processThread.get_id() == std::this_thread::get_id())
        {
            // Avoid joining the thread if it's the same as the current thread
            processThread.detach();
        }
        else
        {
            processThread.join();
        }
    }
    cout << "Flag7\n";
}

// Create a seperate thread that will simulate a running application
void Process::start()
{
    processThread = thread(&Process::run, this);
}

// Running the simulated application and when it ends destroying the thread
void Process::run()
{
    std::this_thread::sleep_for(std::chrono::seconds(executionTime));

    std::function<void(std::shared_ptr<Process>)> callback;
    {
        std::lock_guard<std::mutex> lock(callbackMutex);
        if (onCompleteCallback)
        {
            callback = onCompleteCallback; // Make a local copy
        }
    }

    if (callback)
    {
        auto self = shared_from_this(); // Keep Process alive during callback
        callback(self);                 // Use the local copy
    }
}

//////////////////
// Server class implementation
Server::Server(string instanceTypeInput)
{
    instanceType = instanceTypeInput;
};

// Adds new processes to the server
void Server::launchProcess(int executionTime)
{
    std::lock_guard<std::mutex> lock(processesMutex);
    std::shared_ptr<Process> newProcess = std::make_shared<Process>(executionTime, [this](std::shared_ptr<Process> completedProcess)
                                                                    { removeProcess(completedProcess); });
    activeProcesses.push_back(newProcess);
}

// Removing processes that are executed (This is used as a callback and given to Process class to handle its own lifecycle)
void Server::removeProcess(std::shared_ptr<Process> completedProcess)
{
    cout << "Flag3\n";
    // Then remove from active processes
    std::lock_guard<std::mutex> lock(processesMutex);
    auto it = std::find(activeProcesses.begin(), activeProcesses.end(), completedProcess);
    if (it != activeProcesses.end())
    {
        activeProcesses.erase(it);
    }
    cout << "Process removed successfully\n";
}

// Returning the total amount of processes runnning simultaniously
int Server::getTotalProcessNum()
{
    std::lock_guard<std::mutex> lock(processesMutex);
    return activeProcesses.size();
}
