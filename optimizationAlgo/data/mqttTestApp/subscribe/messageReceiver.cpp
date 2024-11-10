#include <ostream> // std::cout.
#include <fstream> // std::ofstream.
#include <vector>  // vectors.
#include <thread>  // threads.
#include <functional>
#include <sstream>
#include <future>
#include <chrono>
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
    realTimeReportFile = std::make_shared<std::ofstream>(regionName + "_realTime_log", std::ios::trunc);
    endOfDayReportFile = std::make_shared<std::ofstream>(regionName + "_endOfDay_log", std::ios::trunc);
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

            if (messageString == "END OF DAY")
            {
                calculateCostBenefitRatio();
            }

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
            auto newServer = std::make_shared<Server>("c08", [this](std::shared_ptr<Server> serverToChange, int requestedStatus)
                                                      { changeServerType(serverToChange, requestedStatus); });
            serverStatus1.insert(serverStatus1.begin(), newServer);
            ++totalNumOfScaling;
            targetServer = newServer;
        }
    } // serversMutex is released here

    // Now launch the process on the selected server
    if (targetServer)
    {
        targetServer->launchProcess(Constants::averageApplicationExecutionDuration);
        cout << "PROCESS ADDED\n";
        ++totalProcesses;
        // Print out the current server load of the region
        regionalReport();
    }
}

// Adding a new server to the server pool of serverType1 since there is no processes in that server
void RegionalAlgo::addServer(string instanceTypeInput)
{
    ++totalNumOfScaling;
    auto server = std::make_shared<Server>(instanceTypeInput, [this](std::shared_ptr<Server> serverToChange, int requestedStatus)
                                           { changeServerType(serverToChange, requestedStatus); });
    serverStatus1.insert(serverStatus1.begin(), server);
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
            cout << "SERVER CLOSED\n";
            calculateServerCost(serverToChange->elapsed, serverToChange->getInstanceType());
            regionalReport();
        }
        if (requestedStatus == 0)
        {
            serverStatus0.insert(serverStatus0.begin(), serverToChange);
            removeServerFromVector(serverStatus1);
            removeServerFromVector(serverStatus2);
            removeServerFromVector(serverStatus3);
        }
        else if (requestedStatus == 1)
        {
            serverStatus1.insert(serverStatus1.begin(), serverToChange);
            removeServerFromVector(serverStatus0);
            removeServerFromVector(serverStatus2);
            removeServerFromVector(serverStatus3);
        }
        else if (requestedStatus == 2)
        {
            serverStatus2.insert(serverStatus2.begin(), serverToChange);
            removeServerFromVector(serverStatus0);
            removeServerFromVector(serverStatus1);
            removeServerFromVector(serverStatus3);

            // If the proccess amount in the server is increasing then create a new server with increased resource configuration (vertical scaling)
            // and if the server resource is at maximum possible than create an identical server
            if (serverToChange->serverStatus < requestedStatus && serverStatus1.empty())
            {
                auto nextTypeOpt = Constants::getNextInstanceType(serverToChange->getInstanceType());
                if (nextTypeOpt)
                {
                    cout << serverToChange->getInstanceType();
                    cout << *nextTypeOpt;
                    // Optional has a value, so use it
                    addServer(*nextTypeOpt);
                }
                else
                {
                    addServer(serverToChange->getInstanceType());
                }
            }
        }
        else if (requestedStatus == 3)
        {
            serverStatus3.insert(serverStatus3.begin(), serverToChange);
            removeServerFromVector(serverStatus0);
            removeServerFromVector(serverStatus1);
            removeServerFromVector(serverStatus2);
        }
    }
};

void RegionalAlgo::regionalReport()
{
    // Use a stringstream to construct the message
    std::stringstream reportStream;

    // Add to both the reportStream and console output
    reportStream << "Infrastructure update:\n";
    reportStream << "---------------------------\n";
    reportStream << "Total Number of Server Type 0: " << serverStatus0.size() << "\n";
    reportStream << "Total Number of Server Type 1: " << serverStatus1.size() << "\n";
    reportStream << "Total Number of Server Type 2: " << serverStatus2.size() << "\n";
    reportStream << "Total Number of Server Type 3: " << serverStatus3.size() << "\n";
    reportStream << "---------------------------\n";

    // Print individual server details for each status type
    if (!serverStatus0.empty())
    {
        reportStream << "Individual Server Type 0 Process Numbers:\n";
        for (const auto &server : serverStatus0)
        {
            reportStream << server->getInstanceType() << ":" << server->getTotalProcessNum() << "/";
        }
        reportStream << "\n";
    }
    if (!serverStatus1.empty())
    {
        reportStream << "Individual Server Type 1 Process Numbers:\n";
        for (const auto &server : serverStatus1)
        {
            reportStream << server->getInstanceType() << ":" << server->getTotalProcessNum() << "/";
        }
        reportStream << "\n";
    }
    if (!serverStatus2.empty())
    {
        reportStream << "Individual Server Type 2 Process Numbers:\n";
        for (const auto &server : serverStatus2)
        {
            reportStream << server->getInstanceType() << ":" << server->getTotalProcessNum() << "/";
        }
        reportStream << "\n";
    }
    if (!serverStatus3.empty())
    {
        reportStream << "Individual Server Type 3 Process Numbers:\n";
        for (const auto &server : serverStatus3)
        {
            reportStream << server->getInstanceType() << ":" << server->getTotalProcessNum() << "/";
        }
        reportStream << "\n";
    }

    reportStream << "---------------------------\n\n";

    // Output to console
    std::cout << reportStream.str();

    // Output to file if it's open
    if (realTimeReportFile->is_open())
    {
        *realTimeReportFile << reportStream.str();
        realTimeReportFile->flush(); // Ensure the data is written to the file
    }
}

void RegionalAlgo::calculateServerCost(float runTime, string instanceType)
{
    // The server pricing is USD/hour thats why we first find the server runTime in seconds
    // for real life than convert that time to hours and finally multiply with how much that
    // server costs in the region
    float realLifeRunTime = (runTime * 200) / 3600;
    totalServerCost += realLifeRunTime * Constants::overallServerPricing.find(regionName)->second.find(instanceType)->second;
}

void RegionalAlgo::calculateCostBenefitRatio()
{
    // Even tough the server boot might not always be an issue
    // in order to discourage the unnecesarry scale up, in the calculation
    // it is taken as one of the cost factors

    // Use a stringstream to construct the message
    std::stringstream reportStream;

    // Add to both the reportStream and console output
    reportStream << "-------END OF DAY REPORT-------\n";
    reportStream << "Total proccesses that was sent to the server network: " << totalProcesses << endl;
    reportStream << "Total cost to run the server network: " << totalServerCost << "$" << endl;
    reportStream << "Overall time spent on server holdup between scaling and initial boots: " << totalNumOfScaling * Constants::averageServerBootDuration << " seconds"<< endl;
    reportStream << "Maximum vertical availability of the infrastructure: " << Constants::processCapacityPerInstanceType.at("c88").absoluteLimit << endl;
    reportStream << "-------END OF DAY REPORT-------\n";

    // Output to console
    std::cout << reportStream.str();

    // Output to file if it's open
    if (endOfDayReportFile->is_open())
    {
        *endOfDayReportFile << reportStream.str();
        endOfDayReportFile->flush(); // Ensure the data is written to the file
    }
    totalNumOfScaling = 0;
    totalProcesses = 0;
    totalServerCost = 0;
}

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
    start = chrono::steady_clock::now();
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
            auto now = chrono::steady_clock::now();
            elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
            auto self = shared_from_this();             // Keep Process alive during callback
            serverStatusChangeSignalCallback(self, -1); // Use the local copy
            serverStatus = -1;
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
    cout << "PROCESS REMOVED\n";
}

// Returning the total amount of processes runnning simultaniously
int Server::getTotalProcessNum()
{
    std::lock_guard<std::mutex> lock(processesMutex);
    return activeProcesses.size();
}

// Return the instance type of the server
string Server::getInstanceType()
{
    return instanceType;
}
