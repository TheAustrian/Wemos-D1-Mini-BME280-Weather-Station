#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <TimeLib.h> // https://github.com/PaulStoffregen/Time
#include <ESP8266HTTPClient.h>
#include <stdint.h>
#include <BME280.h> // https://github.com/finitespace/BME280
#include "Wire.h"

extern "C" {
#include "user_interface.h"  // needed for light sleep (not really working yet)
}

#include "settings.h"

// Loop & delay variables
unsigned long loopstart = 0;
unsigned long loopend = 0;
long realdelay = 0;
bool justrunonce = true;

// Buffer 
time_t timeBuffer[buffersize] = {0};
float tempBuffer[buffersize] = {0.0};
float pressureBuffer[buffersize] = {0.0};
float humidityBuffer[buffersize] = {0.0};
float voltageBuffer[buffersize] = {0.0};
int bufferposition = 0;

// Include functions after declaring other variables they might use
#include "myfunctions.h"

void setup() {
  Serial.begin(115200);
  wifi_set_sleep_type(LIGHT_SLEEP_T); // LIGHT_SLEEP_T for sleep, NONE_SLEEP_T FOR NO SLEEP
  WiFi.hostname(espname);
  WiFi.mode(WIFI_STA);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    delay(500);
  }
  
wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);

// NTP setup
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  setSyncInterval(1800); //update NTP every 30 mins (in seconds)

// Sensor start  
  mySensor.begin();
}


void loop() {
  loopstart = millis();
  if (justrunonce == true) {
    justrunonce = false;
    // Code that should just run once after start comes here
    // This is to avoid WDT issues in the setup
      delay(10); // sensor needs about 2ms to start up
  }

  // Read sensor data into the buffer
if ( now() >= 100000000 ) {  // Make sure we received the time via NTP, no use reading sensor data without proper timestamp
  for (int i = 0; i <= buffersize ; i++) {
    if ( timeBuffer[bufferposition] == 0 ) {
      break; // Exit the for-loop when we are at an empty buffer position
    } else {
      bufferposition = (bufferposition + 1) % buffersize; // Try the next position. Automagically rolls over if larger than the buffer
   }
  }
  timeBuffer[bufferposition] = now();
  mySensor.ReadData(pressureBuffer[bufferposition], tempBuffer[bufferposition], humidityBuffer[bufferposition], true, 0);   
  voltageBuffer[bufferposition] = analogRead(A0)*voltagemultiplier; 
}

  if(WiFi.status()== WL_CONNECTED)  {
    if ( timeStatus() != timeSet || now() <= 100000000 ) {
      getNtpTime(); // Update NTP time when necessary
    } else {
      if ( SendSensorData("0") == 1 ) { // Test if we can connect to the php page and send data if we can
        delay(1000);
        SendSensorData(SensorData()); // sending up to 30 datasets at once (we can only send about 3000 characters in the POST request)
      }
    }
  
  }

// Go to sleep for the specified time (minus the time we needed for all the stuff we did in the loop)
loopend = millis();
realdelay = sleepdelay + loopstart - loopend;

if ( realdelay > 0 && realdelay < sleepdelay ) {
/*  This doesn't work yet cause there is no wifi_fpm_set_wakeup_cb() yet...
  wifi_station_disconnect(); 
  wifi_set_opmode(NULL_MODE); 
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T); 
  wifi_fpm_open();
  wifi_fpm_set_wakeup_cb(fpm_wakup_cb_func1()); 
  wifi_fpm_do_sleep(realdelay*1000);  */
  delay(realdelay);
  } else {
  delay(0);  
  }
}


