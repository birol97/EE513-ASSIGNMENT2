// Based on the Paho C code example from www.eclipse.org/paho/
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include "MQTTClient.h"
#include <iostream>
#include "ADXL345.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <jsoncpp/json/json.h>
#include <json-c/json.h>
#include <chrono>
#include <ctime> 

#define  CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"
using namespace std;
using namespace exploringRPi;
//Please replace the following address with the address of your server
#define ADDRESS    "tcp://169.254.85.133:1883"
#define CLIENTID   "rpi1"
#define AUTHMETHOD "molloyd"
#define AUTHTOKEN  "12345"
#define TOPIC      "ee513/CPUTemp"
#define QOS        2
#define TIMEOUT    10000L
#define WILL_MESSAGE  "unexpected error"


float getCPUTemperature() {        // get the CPU temperature
   int cpuTemp;                    // store as an int
   fstream fs;
   fs.open(CPU_TEMP, fstream::in); // read from the file
   fs >> cpuTemp;
   fs.close();
   return (((float)cpuTemp)/1000);
}

std::string jsonformat(std::map<std::string,std::vector<float>> x){
cout<<1<<endl;
std::map<std::string,std::vector<float>>::iterator it;
std::string  var = {"\n \"Pitch_roll_values\": " "   "};
int count =0;
for(it =  x.begin();it != x.end(); it++){
count++;

std::string word = it->first;
std::vector<float> values = it->second;
std::string key_pair= "";


for(int z =0; z < values.size();z++){
if(z < values.size()-1){
key_pair =  key_pair    + std::string(to_string(values.at(z))) + "  " + "," + "\n" ;
}
else{
key_pair =  key_pair    + std::string(to_string(values.at(z)))   + "\n" ;

};
};
if(count == 1){
var =  var + '\n' + "\""  + word + "\""+": " + "["+   key_pair + "] ,";
}else{

var =  var + '\n' +"\"" + word + "\""+": " + "["+ '\n' + key_pair + "]";

};
};
var = "{" + var + "}";

return  var;  
 
};




int main(int argc, char* argv[]) {
   char str_payload[6000];
 

  json_object * jobj = json_object_new_object();
  json_object *jstring = json_object_new_string("Unexpected disconnection");
  /*Form the json object*/
    json_object_object_add(jobj,"Last_will_mssage", jstring);
  /*Now printing the json object*/
  printf ("The json object created: %sn",json_object_to_json_string(jobj));  
//  const*  char last_Will_message = json_object_to_json_string(jobj);
           // Set your max message size here
   MQTTClient client;
   MQTTClient_willOptions  willOptions = MQTTClient_willOptions_initializer;
        
	willOptions.topicName = "ee513/CPUTemp";
	willOptions.message = json_object_to_json_string(jobj);
;
	willOptions.retained = 0;
	willOptions.qos = QOS;
        
  MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
 MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken token; 
   MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE,NULL);
 opts.keepAliveInterval = 20; 
opts.cleansession = 0;
 opts.username = AUTHMETHOD; 
opts.password = 
   AUTHTOKEN;
opts.will = &willOptions;    
   ADXL345 sensor(1,0x53);
   sensor.setResolution(ADXL345::NORMAL);
   sensor.setRange(ADXL345::PLUSMINUS_4_G);
   std::vector<float>  x  = sensor.displayPitchAndRoll();
     
  // std::map<std::string,std::vector<float>>  x  = sensor.displayPitchAndRoll();  // x is map that has the sensor_values
 //  std::string xxx =  jsonformat(x); //sent to json format code to  format in json
   auto start = std::chrono::system_clock::now();
   int rc;
   if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {

      cout << "Failed to connect, return code " << rc << endl;
      return -1;
   }
   char* pitch_and_roll = new char[4200];
//   strcpy(pitch_and_roll,xxx.c_str());
  // current date/time based on current system
   time_t now = time(0);
   
   // convert now to string form
   char* dt = ctime(&now);
   
   char z[20] = "dsdsdsds";
  
 //sprintf(str_payload, "{\"d\":{\"CPUTemp\": %f },\"d\":{\"CPUTemp\": %f }} ",
// getCPUTemperature(),getCPUTemperature());
   int count= 0;
  while(count <100){
   
   ADXL345 sensor(1,0x53);
   sensor.setResolution(ADXL345::NORMAL);
   sensor.setRange(ADXL345::PLUSMINUS_4_G);

    count++;
    
    std::vector<float>  x  = sensor.displayPitchAndRoll();
    time_t now = time(0);

   // convert now to string form
   char* dt = ctime(&now);
    
  // sprintf(str_payload, """{\"d\":{\"CPUTemp\": %f  }, %s  ""\}", getCPUTemperature(),pitch_and_roll);
   sprintf(str_payload, " {\"CPUTemp\": %f ,\"Pitchvalues\": [%f] ,\"Rollvalues\": [%f],\"Time\":""\" %s ""\",\"Time\":""\" %d ""\"  }   ",getCPUTemperature() ,x.at(0),x.at(1),dt,count);
   pubmsg.payload = str_payload;
   pubmsg.payloadlen = strlen(str_payload);
   pubmsg.qos = QOS;
   pubmsg.retained = 0;
 
   MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
   cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
        " seconds for publication of " << str_payload <<
        " \non topic " << TOPIC << " for ClientID: " << CLIENTID << endl;
   rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
   cout << "Message with token " << (int)token << " delivered." << endl;
 }
   MQTTClient_disconnect(client, 10000);
   MQTTClient_destroy(&client);
    return rc;
}
