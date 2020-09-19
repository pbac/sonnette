#include <Arduino.h>
#include <ESP8266WiFi.h>          // ESP8266 Core WiFi Library 

//#include <ESP8266httpClient.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <math.h>

#include "sonnette.h"
#include "secret.h"                // Store all private info (SECRET_*)

// WIFI connection
const char*     ssid            = SECRET_WIFI_SSID;            // The SSID (name) of the Wi-Fi network you want to connect to
const char*     password        = SECRET_WIFI_PASSWORD;       // The password of the Wi-Fi network
char            localIp[20];

// MQTT server connection
const char*     mqttServer      = SECRET_MQTT_SERVERNAME;
const int       mqttPort        = 1883;
const char*     mqttUser        = SECRET_MQTT_USER;
const char*     mqttPassword    = SECRET_MQTT_PASSWORD;
const char*     deviceName      = "node130";

//-------------------------------------------------------------------

WiFiClient      wifiCnx;
PubSubClient    mqttCnx(wifiCnx);

//-------------------------------------------------------------------
bool led    	= false;
int  iLoop		= 0; 	
bool sonnette 	= 0;
char mqttBuffer[256];


//------------------------  M Q T T   C O N N E C T I O N  --------------------------------------
void mqttCallback(char* topic, byte* payload, unsigned int length) {

    unsigned int    i;

    debugln("Message recu =>  topic: " + String(topic));
    debug(" | longueur: " + String(length,DEC));

    for(i=0; i<length; i++) {                                                   // create character buffer with ending null terminator (string)
        mqttBuffer[i] = payload[i];
    }
    mqttBuffer[i] = '\0';

    String msgString = String(mqttBuffer);
    debugln("Payload: " + msgString);
}

void mqttConnect() {

    while (!mqttCnx.connected()) {
        if (mqttCnx.connect(deviceName, mqttUser, mqttPassword)) {
            debugln("MQTT connexion OK");
        } else {
            debug("MQTT Connexion failed with state ");
            debug(mqttCnx.state());
            delay(1000);
        }
    }

}

void mqttSend(const char* category, const char* label, char* value) {

    mqttConnect();
    String topic = String(deviceName) + "/sensor/" + String(category) + "/" + String(label);
    mqttCnx.publish(topic.c_str(), String(value).c_str(), false); 

    debugln("MQTT " + String(topic) + ": " + String(value));
}

void mqttSendValue(char* category, int sensor, char unit, float value) {
	char aValue[32];
	sprintf(aValue, "%.2f", value);
	String cCategory = String(category) + String(sensor);
	mqttSend(cCategory.c_str(), String(unit).c_str(), aValue);
}



//------------------------ S E T U P --------------------------------------
void setup() {
    int     i = 0;
    
    // SERIAL
    Serial.begin(115200);
    debugln("Starting setup");

    //PIN
    pinMode(LED_BUILTIN, OUTPUT);                                           // set led pin as output
	pinMode(SONNETTE_PIN, INPUT);
	pinMode(POWERLED_PIN, OUTPUT);
	pinMode(EVENTLED_PIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);                                      
	digitalWrite(POWERLED_PIN, HIGH);                                 

    // WIFI Connection
    WiFi.begin(ssid, password);         
    debug(VERSION);
    debug(" Connecting to ");
    debug(ssid); 
    debugln(" ...");

    while (WiFi.status() != WL_CONNECTED) { // Wait (4min max) for the Wi-Fi to connect
        delay(500);
        debug(++i); 
        debug(' ');
        if (i > 500) {                      // Reboot if no wifi connection 
            ESP.reset();
        }
    }

    // Local IP Copy
    String sLocalIp = String() + WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3];
    strcpy(localIp,sLocalIp.c_str());

    debugln('\n');
    debugln("Connection established!");  
    debug("IP address:\t");
    debugln(localIp);         // Send the IP address of the ESP8266 to the computer

	//MQTT
    debugln("setting mqtt server to " + String(mqttServer));   
    mqttCnx.setServer(mqttServer, 1883);                                      //Configuration de la connexion au serveur MQTT
    mqttCnx.setCallback(mqttCallback);                                        //La fonction de callback qui est executée à chaque réception de message  
    mqttCnx.disconnect();
    mqttConnect();
    mqttSend("IP", "started", localIp);
								

}

//------------------------ L O O P --------------------------------------

void loop() {

	bool   	thisSonnette;
	char	mqttState[10];	
	iLoop++;

	thisSonnette = !digitalRead(SONNETTE_PIN);

	if (sonnette != thisSonnette || iLoop >= 2000) {
		sonnette = thisSonnette;
		iLoop = 0;
		if (sonnette) {
			strcpy(mqttState, "1");
		} else {
			strcpy(mqttState, "0");
		}
		digitalWrite(EVENTLED_PIN, sonnette);                                         // keep LED off
		mqttSend("event", "sonnette", mqttState);
		mqttCnx.loop();
		mqttCnx.disconnect();
	}

	if ((iLoop % 20) == 0 || sonnette != 0) {
		led = !led;
		digitalWrite(LED_BUILTIN, led);                                          // keep LED off
		digitalWrite(POWERLED_PIN, led);                                         // keep LED off
	}

	// If Wifi lost then reset the ESP
	if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wifi connexion lost : Rebooting ESP");
		delay(5000);
		ESP.reset();
	}
	delay(50);
}
