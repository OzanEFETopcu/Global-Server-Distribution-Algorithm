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

        if (serverStatus0.size() > 0)
        {
            targetServer = serverStatus0.front();
        }
        else if (serverStatus1.size() > 0)
        {
            targetServer = serverStatus1.front();
        }
        else if (serverStatus2.size() > 0)
        {
            targetServer = serverStatus2.front();
        }
        else
        {
            // Need to add a new server
            auto newServer = std::make_shared<Server>("c8", [this](std::shared_ptr<Server> serverToChange, int requestedStatus)
                                                      { changeServerType(serverToChange, requestedStatus); });
            serverStatus1.push_back(newServer);
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
        cout << "Server Type 0 Amount: " << serverStatus0.size() << endl;
        cout << "Server Type 1 Amount: " << serverStatus1.size() << endl;
        cout << "Server Type 2 Amount: " << serverStatus2.size() << endl;
        cout << "Server Type 3 Amount: " << serverStatus3.size() << endl;
        cout << "---------------------------\n";
        if (!serverStatus0.empty())
        {
            cout << "Server Type 0 Process Num: " << serverStatus0.front()->getTotalProcessNum() << endl;
        }
        if (!serverStatus1.empty())
        {
            cout << "Server Type 1 Process Num: " << serverStatus1.front()->getTotalProcessNum() << endl;
        }
        if (!serverStatus2.empty())
        {
            cout << "Server Type 2 Process Num: " << serverStatus2.front()->getTotalProcessNum() << endl;
        }
        if (!serverStatus3.empty())
        {
            cout << "Server Type 3 Process Num: " << serverStatus3.front()->getTotalProcessNum() << endl;
        }
        cout << "---------------------------\n\n\n\n";
    }
}

// Adding a new server to the server pool of serverType1 since there is no processes in that server
void RegionalAlgo::addServer(string instanceTypeInput)
{
    std::lock_guard<std::mutex> lock(serversMutex);
    auto server = std::make_shared<Server>(instanceTypeInput, [this](std::shared_ptr<Server> serverToChange, int requestedStatus)
                                           { changeServerType(serverToChange, requestedStatus); });
    serverStatus1.push_back(server);
};

// Removing servers that are no more used
void RegionalAlgo::removeServer()
{
    std::lock_guard<std::mutex> lock(serversMutex);
    // Remove servers with no active processes from the serverType1 vector
    serverStatus1.erase(
        std::remove_if(serverStatus1.begin(), serverStatus1.end(),
                       [](const std::shared_ptr<Server> &server)
                       {
                           return server->getTotalProcessNum() == 0;
                       }),
        serverStatus1.end());
};

// Changing server's vector from one type to another depending on its occupancy
void RegionalAlgo::changeServerType(std::shared_ptr<Server> serverToChange, int requestedStatus)
{

    std::lock_guard<std::mutex> lock(serversMutex);

    // Remove server from the original vector it's in
    auto removeServerFromVector = [&](vector<std::shared_ptr<Server>> &sourceVector)
    {
        auto it = std::remove(sourceVector.begin(), sourceVector.end(), serverToChange);
        if (it != sourceVector.end())
        {
            sourceVector.erase(it);
        }
    };

    if (serverToChange->serverStatus != requestedStatus)
    {
        if (requestedStatus == -1)
        {
            removeServerFromVector(serverStatus0);
            removeServerFromVector(serverStatus1);
            removeServerFromVector(serverStatus2);
            removeServerFromVector(serverStatus3);
        }
        if (requestedStatus == 0)
        {
            serverStatus0.push_back(serverToChange);
            removeServerFromVector(serverStatus1);
            removeServerFromVector(serverStatus2);
            removeServerFromVector(serverStatus3);
        }
        else if (requestedStatus == 1)
        {
            serverStatus1.push_back(serverToChange);
            removeServerFromVector(serverStatus0);
            removeServerFromVector(serverStatus2);
            removeServerFromVector(serverStatus3);
        }
        else if (requestedStatus == 2)
        {
            serverStatus2.push_back(serverToChange);
            removeServerFromVector(serverStatus0);
            removeServerFromVector(serverStatus1);
            removeServerFromVector(serverStatus3);
        }
        else if (requestedStatus == 3)
        {
            serverStatus3.push_back(serverToChange);
            removeServerFromVector(serverStatus0);
            removeServerFromVector(serverStatus1);
            removeServerFromVector(serverStatus2);
        }
    }
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
Server::Server(string instanceTypeInput, function<void(std::shared_ptr<Server> serverToChange, int requestedStatus)> serverStatusChangeSignal)
{
    serverStatusChangeSignalCallback = serverStatusChangeSignal;
    instanceType = instanceTypeInput;
    serverStatus = 1;
};

// Adds new processes to the server
void Server::launchProcess(int executionTime)
{
    std::lock_guard<std::mutex> lock(processesMutex);
    std::shared_ptr<Process> newProcess = std::make_shared<Process>(executionTime, [this](std::shared_ptr<Process> completedProcess)
                                                                    { removeProcess(completedProcess); });
    activeProcesses.push_back(newProcess);
    changeStatus();
}

// Send a callback to the algorithm for updating server status according to active process num
void Server::changeStatus()
{
    if (activeProcesses.size() == 0)
    {
        if (serverStatusChangeSignalCallback)
        {
            auto self = shared_from_this();            // Keep Process alive during callback
            serverStatusChangeSignalCallback(self, -1); // Use the local copy
        }
    }
    else if (activeProcesses.size() <= Constants::processCapacityPerInstanceType.at(instanceType).minThreshold)
    {
        if (serverStatusChangeSignalCallback)
        {
            auto self = shared_from_this();            // Keep Process alive during callback
            serverStatusChangeSignalCallback(self, 1); // Use the local copy
            serverStatus = 1;
        }
    }
    else if (activeProcesses.size() <= Constants::processCapacityPerInstanceType.at(instanceType).maxThreshold)
    {
        if (serverStatusChangeSignalCallback)
        {
            auto self = shared_from_this();            // Keep Process alive during callback
            serverStatusChangeSignalCallback(self, 0); // Use the local copy
            serverStatus = 0;
        }
    }
    else if (activeProcesses.size() < Constants::processCapacityPerInstanceType.at(instanceType).absoluteLimit)
    {
        if (serverStatusChangeSignalCallback)
        {
            auto self = shared_from_this();            // Keep Process alive during callback
            serverStatusChangeSignalCallback(self, 2); // Use the local copy
            serverStatus = 2;
        }
    }
    else if (activeProcesses.size() == Constants::processCapacityPerInstanceType.at(instanceType).absoluteLimit)
    {
        if (serverStatusChangeSignalCallback)
        {
            auto self = shared_from_this();            // Keep Process alive during callback
            serverStatusChangeSignalCallback(self, 3); // Use the local copy
            serverStatus = 3;
        }
    }
    else
    {
        cout << "!!!!ERROR ON SERVER STATUS CHANGE!!!!" << endl;
    }
};

// Removing processes that are executed (This is used as a callback and given to Process class to handle its own lifecycle)
void Server::removeProcess(std::shared_ptr<Process> completedProcess)
{
    // Then remove from active processes
    std::lock_guard<std::mutex> lock(processesMutex);
    auto it = std::find(activeProcesses.begin(), activeProcesses.end(), completedProcess);
    if (it != activeProcesses.end())
    {
        activeProcesses.erase(it);
    }
    changeStatus();
    cout << "Process removed successfully\n";
}

// Returning the total amount of processes runnning simultaniously
int Server::getTotalProcessNum()
{
    std::lock_guard<std::mutex> lock(processesMutex);
    return activeProcesses.size();
}
