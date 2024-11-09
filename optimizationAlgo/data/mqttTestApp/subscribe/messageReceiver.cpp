#include <ostream> // std::cout.
#include <vector>  // vectors.
#include <thread>  // threads.
#include <functional>
#include <future>
#include "mqttSubscribeMessage.h"
#include "messageReceiver.h"
using namespace std;

//////////////////
// Regional algorithm class implementation
RegionalAlgo::RegionalAlgo(string regionNameInput)
{
    regionName = regionNameInput;
}

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
            cout << "Hello Mate!!\n";
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
    if (serverType0.size() > 0)
    {
        serverType0.front()->launchProcess(60);
    }
    else if (serverType1.size() > 0)
    {
        serverType1.front()->launchProcess(60);
        cout << to_string(serverType1.size()) << endl;
        cout << to_string(serverType1.front()->getTotalProcessNum()) << endl;
    }
    else if (serverType2.size() > 0)
    {
        serverType2.front()->launchProcess(60);
    }
    else
    {
        addServer("c4");
        // Since the only server is in serverType1 the first value of the vector is our server
        serverType1.front()->launchProcess(60);
    }
}

// Adding a new server to the server pool of serverType1 since there is no processes in that server
void RegionalAlgo::addServer(string instanceTypeInput)
{
    std::shared_ptr<Server> server = std::make_shared<Server>(instanceTypeInput);
    serverType1.push_back(server);
};

// Removing servers that are no more used
void RegionalAlgo::removeServer()
{
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

// Create a seperate thread that will simulate a running application
void Process::start()
{
    processThread = thread(&Process::run, this);
}

// Running the simulated application and when it ends destroying the thread
void Process::run()
{
    std::this_thread::sleep_for(std::chrono::seconds(executionTime));

    // Notify that this process is complete
    if (onCompleteCallback)
    {
        onCompleteCallback(shared_from_this());
    }
}

//////////////////
// Server class implementation
Server::Server(string instanceTypeInput)
{
    instanceType = instanceTypeInput;
}

// Adds new processes to the server
void Server::launchProcess(int executionTime)
{
    std::shared_ptr<Process> newProcess = std::make_shared<Process>(executionTime, [this](std::shared_ptr<Process> completedProcess)
                                                                    { this->removeProcess(completedProcess); });
    activeProcesses.push_back(newProcess);
}

// Removing processes that are executed (This is used as a callback and given to Process class to handle its own lifecycle)
void Server::removeProcess(std::shared_ptr<Process> completedProcess)
{
    auto it = std::find(activeProcesses.begin(), activeProcesses.end(), completedProcess);
    if (it != activeProcesses.end())
    {
        activeProcesses.erase(it);
        if (completedProcess->processThread.joinable())
        {
            completedProcess->processThread.join(); // Join the thread here
        }
    }
}

// Returning the total amount of processes runnning simultaniously
int Server::getTotalProcessNum()
{
    return activeProcesses.size();
}
