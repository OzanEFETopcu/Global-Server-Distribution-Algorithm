#include <mqtt/client.h>
using namespace std;

unique_ptr<mqtt::client> initiatePubClient(const string &id)
{
    // In order to connect the mqtt client to a broker,
    // Define an Ip address pointing to a broker. In this case, the localhost on port 1883.
    string ip = "localhost:1883";
    // Construct a client using the Ip and Id, specifying usage of MQTT V5.
    auto client = make_unique<mqtt::client>(ip, id, mqtt::create_options(MQTTVERSION_5));
    client->connect();
    return client;
}

void publishMessage(const string &payload, mqtt::client &client, mqtt::message_ptr &message)
{
    // Configure Mqtt message to contain payload specifying time until end.
    message->set_payload(payload);

    // Publish the Mqtt message using the connected client.
    client.publish(message);
}