To compile the request generator:
g++ mainRequestCenter.cpp requestGenerator.cpp mqttPublishMessage.cpp -lpaho-mqttpp3 -lpaho-mqtt3as -lpthread -o requestGenerator

To compile the simple consumer:
g++ SimpleMqttConsumer.cpp -lpaho-mqttpp3 -lpaho-mqtt3as -o simpleConsumer

!!!Dont forget to first export the environmental variables, its command is given in envVars.txt file!!!