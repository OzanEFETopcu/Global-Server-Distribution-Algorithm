#ifndef MY_CLASS_H
#define MY_CLASS_H
using namespace std;

unique_ptr<mqtt::client> initiateClient(const std::string &id);
void publishMessage(const string &payload, mqtt::client &client, mqtt::message_ptr &message);

#endif