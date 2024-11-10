#ifndef MESSAGE_RECEIVER
#define MESSAGE_RECEIVER
#include <string>
#include <thread>
#include <vector>
#include <fstream>
#include <functional>
#include <mutex>
using namespace std;

class Process : public std::enable_shared_from_this<Process>
{
public:
    Process(int executionTimeInput, function<void(std::shared_ptr<Process>)> onComplete);
    ~Process();
    void start();
    void run();
    thread processThread;

private:
    int executionTime;
    function<void(std::shared_ptr<Process> completedProces)> onCompleteCallback;
    std::mutex mutex_;
    std::mutex callbackMutex;
};

class Server : public std::enable_shared_from_this<Server>
{
public:
    Server(string instanceTypeInput, function<void(std::shared_ptr<Server> serverToChange, int requestedStatus)> serverStatusChangeSignal);
    void launchProcess(int executionTime);
    void removeProcess(std::shared_ptr<Process> completedProcess);
    void changeStatus();
    int getTotalProcessNum();
    string getInstanceType();
    int serverStatus;

private:
    function<void(std::shared_ptr<Server> serverToChange, int requestedStatus)> serverStatusChangeSignalCallback;
    std::mutex processesMutex;
    string instanceType;
    vector<std::shared_ptr<Process>> activeProcesses;
};

class RegionalAlgo
{
public:
    RegionalAlgo(string regionNameInput);
    string regionName;
    void messageReceiver();
    void addProcessToServer();
    void addServer(string instanceTypeInput);
    void removeServer();
    void changeServerType(std::shared_ptr<Server> serverToChange, int requestedType);

    // Monitoring of algorithm changes
    void regionalReport();

private:
    std::shared_ptr<std::ofstream> file_ptr;
    std::mutex serversMutex;
    // Servers that have # of processes between min and max thresholds
    vector<std::shared_ptr<Server>> serverStatus0;
    // Servers that have # of processes smaller than the min threshold
    vector<std::shared_ptr<Server>> serverStatus1;
    // Servers that have # of processes larger than the max threshold
    vector<std::shared_ptr<Server>> serverStatus2;
    // Servers that have maximum possible # of processes
    vector<std::shared_ptr<Server>> serverStatus3;
};

#endif