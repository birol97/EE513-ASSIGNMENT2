#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"
#include <jsoncpp/json/json.h>
#include <json-c/json.h>

#define ADDRESS     "tcp://169.254.85.133:1883"
#define CLIENTID    "rpi3"
#define AUTHMETHOD  "molloyd"
#define AUTHTOKEN   "12345"
#define TOPIC       "ee513/CPUTemp"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L
#define GPIO        "/sys/class/gpio/"
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
using namespace std;
   
volatile MQTTClient_deliveryToken deliveredtoken;


void turnlightof(){

cout << "Turning of the led"<<  endl;
int gpioNumber = 16;
string gpioPath = string(GPIO "gpio") + to_string(gpioNumber)+ string("/");
ofstream fs;
fs.open((string(GPIO)+"export").c_str());
fs <<  "to_string(gpioNumber))";
fs.close();
usleep(100000);

fs.open((gpioPath+"direction").c_str());
fs << "out";
fs.close();
usleep(100000);

fs.open((gpioPath+"value").c_str());
fs << "1";
fs.close();


};


void turnlighton(){

cout << "Turning on the led"<<  endl;
int gpioNumber = 16;
string gpioPath = string(GPIO "gpio") + to_string(gpioNumber)+ string("/");
ofstream fs;
fs.open((string(GPIO)+"export").c_str());
fs <<  "to_string(gpioNumber))";
fs.close();
usleep(100000);

fs.open((gpioPath+"direction").c_str());
fs << "out";
fs.close();
usleep(100000);

fs.open((gpioPath+"value").c_str());
fs << "0";
fs.close();

};

void json_parse2(json_object * jobj){
struct json_object *CPUTemp;  //creating json object for the each json object that is received
struct json_object *parsed_pitch;
struct json_object *parsed_pitch_values;
struct json_object *parsed_roll;
struct json_object *temp;  // temporary json_object for comparing the value to the local threshold.
size_t n_pitchvalues;
size_t n_rollvalues;


json_object_object_get_ex(jobj, "CPUTemp", &CPUTemp);
json_object_object_get_ex(jobj, "Rollvalues", &parsed_roll);
n_rollvalues = json_object_array_length(parsed_roll);
for(int i = 0;i < n_rollvalues;i++){
temp = json_object_array_get_idx(parsed_roll,i);
if(json_object_get_double(temp) > 1){
   turnlighton();
 }
else{

   turnlightof();

};

};

};

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    int i;
    char* payloadptr;
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");
    payloadptr = (char*) message->payload;
    json_object * jobj = json_tokener_parse(payloadptr);
    json_parse2(jobj);
    for(i=0; i<message->payloadlen; i++) {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}



void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);

    do {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
