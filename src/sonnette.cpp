#include <Arduino.h>
#include <ESP8266WiFi.h>          // ESP8266 Core WiFi Library 

#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <math.h>

#include "main.h"
#include "secret.h"
#include "mqtt54.h"

#define		DEVICE_TYPE			"node"
#define		DEVICE_ID			"130"
#define		VERSION				"2.3.2"

#define 	BUILTIN_LED			16
#define 	SONNETTE_PIN		4
#define 	POWERLED_PIN		14      //WS2812 led
#define 	EVENTLED_PIN		5


//-------------------------------------------------------------------

WiFiClient      wifiCnx;
Mqtt54			mqttCnx(wifiCnx, SECRET_MQTT_SERVERNAME, SECRET_MQTT_PORT, SECRET_MQTT_USER, SECRET_MQTT_PASSWORD);

//-------------------------------------------------------------------
bool sonnette 	= 0;

//_______________________________________________________________________________________________________
//_______________________________________________________________________________________________________

void flashLed () {
	static bool	builtIn_led = 0;		// For flashing the ESP led

	builtIn_led = !builtIn_led;
	digitalWrite(BUILTIN_LED, builtIn_led);
	digitalWrite(POWERLED_PIN, builtIn_led);

}
void flashLed (bool	builtIn_led) {
	digitalWrite(BUILTIN_LED, builtIn_led);
	digitalWrite(POWERLED_PIN, builtIn_led);
}

//___________________________________  W I F I   C O N N E C T I O N  ___________________________________
//_______________________________________________________________________________________________________

int wifiConnection(const char * Ssid, const char * Password, const char * Hostname) {
	int		cnxWait = 0;

	flashLed(1);
	WiFi.mode(WIFI_STA);											// Set atation mode only (not AP)
	delay(150);
	//WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);			// Reset all address
	delay(150);
	WiFi.hostname(Hostname);										// Set the hostname (for dhcp server)
	delay(150);
	WiFi.begin(Ssid, Password);										// Connect to the network
	debug(String() + Hostname + " connecting to " + Ssid + "... "); 

	while (WiFi.status() != WL_CONNECTED) {						// Wait (4min max) for the Wi-Fi to connect
		delay(500);
		debug(String(++cnxWait) + "."); 
		if (cnxWait > 500) {ESP.restart();}							// Reboot if no wifi connection 
	}
	flashLed(0);
	debugln();
	debugln(String() + "IP address  :\t" + WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3]);
	return cnxWait;
}

//------------------------ S E T U P --------------------------------------
void setup() {
	
	// SERIAL
	Serial.begin(115200);
	debugln("Starting " DEVICE_TYPE " " DEVICE_ID " v" VERSION);

	//PIN
	pinMode(BUILTIN_LED, OUTPUT);                                           // set led pin as output
	pinMode(SONNETTE_PIN, INPUT);
	pinMode(POWERLED_PIN, OUTPUT);
	pinMode(EVENTLED_PIN, OUTPUT);
	digitalWrite(BUILTIN_LED, LOW);                                      
	digitalWrite(POWERLED_PIN, HIGH);                                 

	// WIFI Connection
	wifiConnection(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD, DEVICE_TYPE DEVICE_ID);
	mqttCnx.setDevice(DEVICE_TYPE, DEVICE_ID);
	mqttCnx.setTime(SECRET_NTP_SERVERNAME, SECRET_NTP_TIMEZONE);
	mqttCnx.start(WiFi.localIP(), WiFi.macAddress());
	mqttCnx.send("device", "version",  "node", VERSION);	

}

//------------------------ L O O P --------------------------------------

void loop() {

	static bool		thisSonnette;
	static char		mqttState[10];	
	static int 		iLoop = 0;

	mqttCnx.loop();

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
		digitalWrite(EVENTLED_PIN, sonnette);
		mqttCnx.send("event", "sonnette", "state", mqttState);
	}

	if ((iLoop % 20) == 0 || sonnette != 0) {
		flashLed();
	}

	if (WiFi.status() != WL_CONNECTED) {
		Serial.println("Wifi connexion lost : Rebooting ESP");
		delay(5000);
		ESP.restart();
	}
	delay(50);
}
