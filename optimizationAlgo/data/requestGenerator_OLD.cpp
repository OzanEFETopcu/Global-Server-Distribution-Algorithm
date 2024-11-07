#include <iostream>
#include <chrono>
#include <iomanip>
#include <thread>
#include <future>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
using namespace std;

void _createRequests();

void simulateDay()
{
    const int minutesInADay = 1440;
    const int scaleFactor = 200;
    const int simulationIntervalMs = 60 * 1000 / scaleFactor; // Milliseconds for each simulated seconds ( 0.3 seconds = 1 minute in real life)
    srand(time(0));

    while (true)
    {
    //  Set initial simulation time to 00:00:00 (midnight)
    int simulatedMinutes = 0;
    _createRequests();
    while (simulatedMinutes < minutesInADay)
    {
        // Calculate hours, minutes, and seconds for simulated time
        int hours = (simulatedMinutes / 60) % 24;
        int minutes = simulatedMinutes % 60;

        // Print out simulated time
        //cout << setfill('0') << setw(2) << hours << ":"
        //     << setfill('0') << setw(2) << minutes << "\n";

        // Wait for the interval to pass (0.3s simulates 1 minute in scaled time)
        this_thread::sleep_for(chrono::milliseconds(simulationIntervalMs));

        //future ft = async(_publishRequests);

        // Increment simulated time
        simulatedMinutes++;
    }
    }
};

void _publishRequests() {};

void _createRequests()
{
    int _highTrafficReqNum;
    int _lowTrafficReqNum;
    int _minRange;
    int _maxRange;

    // Deciding how many request will be sent for the day (for both high and low traffic hours)
    // Between low traffic hours total of requests are assumed to be between 0 and 100 whereas 
    _minRange = 0;
    _maxRange = 100;
    _lowTrafficReqNum = (_minRange + rand() % ((_maxRange - _minRange) + 1));
    _lowTrafficReqNum = _lowTrafficReqNum - (_lowTrafficReqNum % 2);
    _minRange = 250;
    _maxRange = 350;
    _highTrafficReqNum = _minRange + rand() % ((_maxRange - _minRange) + 1);
    _highTrafficReqNum = _highTrafficReqNum - (_highTrafficReqNum % 2);

    // Deciding when exactly will the requests be sent
    vector<string> _lowTrafficRequestTimes(_lowTrafficReqNum / 2, "");
    vector<string> _highTrafficRequestTimes(_highTrafficReqNum / 2, "");
    for (int i = 0; i < _lowTrafficReqNum / 2; i++)
    {
        int _requestHours = 16 + rand() % ((31 - 16) + 1);
        if (_requestHours > 23)
        {
            _requestHours = _requestHours - 24;
        };

        int _requestMinutes = 0 + rand() % ((59 - 0) + 1);
        _lowTrafficRequestTimes[i] = to_string(_requestHours) + ":" + to_string(_requestMinutes);
    }

    for (int i = 0; i < _highTrafficReqNum / 2; i++)
    {
        int _requestHours = 8 + rand() % ((15 - 8) + 1);
        int _requestMinutes = 0 + rand() % ((59 - 0) + 1);
        _highTrafficRequestTimes[i] = to_string(_requestHours) + ":" + to_string(_requestMinutes);
    }


    //Debugging
    cout << "High Traffic " << _highTrafficReqNum << "\n";
    for (int i = 0; i < _highTrafficRequestTimes.size(); i++)
    {
        cout << "High Traffic Request Times" << i << "= " << _highTrafficRequestTimes[i]<< "\n";
    }

    cout << "Low Traffic " << _lowTrafficReqNum<< "\n";
    for (int i = 0; i < _lowTrafficRequestTimes.size(); i++)
    {
        cout << "Low Traffic Request Times" << i << "= " << _lowTrafficRequestTimes[i]<< "\n";
    }
};