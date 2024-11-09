#ifndef MQTT_SUBSCRIBE_MESSAGE
#define MQTT_SUBSCRIBE_MESSAGE

#include <memory>        // For std::unique_ptr
#include <string>        // For std::string
#include <mqtt/client.h> // For mqtt::client

using namespace std;

unique_ptr<mqtt::client> initiateSubClient(const string &id, const string &topicName);

#endif