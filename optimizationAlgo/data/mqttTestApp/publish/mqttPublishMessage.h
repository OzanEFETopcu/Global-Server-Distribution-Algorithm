#ifndef MQTT_PUBLISH_MESSAGE
#define MQTT_PUBLISH_MESSAGE

#include <memory>        // For std::unique_ptr
#include <string>        // For std::string
#include <mqtt/client.h> // For mqtt::client and mqtt::message_ptr

using namespace std;

unique_ptr<mqtt::client> initiatePubClient(const string &id);
void publishMessage(const string &payload, mqtt::client &client, mqtt::message_ptr &message);

#endif