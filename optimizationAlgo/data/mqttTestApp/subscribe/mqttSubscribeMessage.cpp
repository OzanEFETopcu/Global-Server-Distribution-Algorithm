// Start by `#include`-ing the Mosquitto MQTT Library and other standard libraries.
#include <mqtt/client.h> // Mosquitto client.
using namespace std;

unique_ptr<mqtt::client> initiateSubClient(const string &id, const string &topicName)
{
    // In order to connect the mqtt client to a broker,
    // Define an Ip address pointing to a broker. In this case, the localhost on port 1883.
    string ip = "localhost:1883";
    // Then, define an ID to be used by the client when communicating with the broker.

    // Construct a client using the Ip and Id, specifying usage of MQTT V5.
    auto client = make_unique<mqtt::client>(ip, id, mqtt::create_options(MQTTVERSION_5));
    // Use the connect method of the client to establish a connection to the broker.
    client->connect();
    // In order to receive messages from the broker, specify a topic to subscribe to.
    client->subscribe(topicName);
    // Begin the client's message processing loop, filling a queue with messages.
    client->start_consuming();

    return client;
}
