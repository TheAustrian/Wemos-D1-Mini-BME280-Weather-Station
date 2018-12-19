#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <TimeLib.h> // https://github.com/PaulStoffregen/Time
#include <ESP8266HTTPClient.h>
#include <stdint.h>
#include <BME280I2C.h> // https://github.com/finitespace/BME280
#include "Wire.h"
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

extern "C"
{
  #include "user_interface.h"  // needed for light sleep (not really working yet)
}

#include "settings.h"

// Loop & delay variables
unsigned long loopstart = 0;
unsigned long loopend = 0;
unsigned long realdelay = 0;
unsigned long worktime = 0;
bool justRunOnce = true;

// Buffer 
time_t timeBuffer[BUFFER_SIZE] = {0};
float tempBuffer[BUFFER_SIZE] = {0.0};
float pressureBuffer[BUFFER_SIZE] = {0.0};
float humidityBuffer[BUFFER_SIZE] = {0.0};
float voltageBuffer[BUFFER_SIZE] = {0.0};
int bufferposition = 0;

// Include functions after declaring other variables they might use
#include "myfunctions.h"

void setup()
{
  #ifdef DEEP_SLEEP
    // In case of deep sleep, initialization is also part of the "loop" what we want to know how long it takes with the actual measurement altogether
    loopstart = millis();
  #endif
  
  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);
  
  delay(1000);  // to make sure Arduino serial monitor is ready to receive messages
  
  // Enable watchdog - https://folk.uio.no/jeanra/Microelectronics/ArduinoWatchdog.html
  wdt_enable(WDTO_8S);
  
  // Initialize buffers - Is this really needed???
  for (int i = 0; i++; i < BUFFER_SIZE)
  {
    timeBuffer[i] = 0;
    tempBuffer[i] = 0.0;
    pressureBuffer[i] = 0.0;
    humidityBuffer[i] = 0.0; 
    voltageBuffer[i] = 0.0;
    
    wdt_reset();
  }

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  #ifdef SERIAL
    setupSerial();
  #endif
  
  setupWiFi();
  setupNtp();
  setupSensor();
  
  #ifdef OTA
    setupOta();
  #endif

  mlog(S_DEBUG, "Collector URL: " + String(DATA_REST_ENDPOINT));
  #ifdef DEEP_SLEEP
    mlog(S_DEBUG, "Deep sleep is truned ON");
  #else
    mlog(S_DEBUG, "Deep sleep is turned OFF");
  #endif
  
  mlog(S_INFO, "Initialization done");
}

#ifdef SERIAL
void setupSerial()
{
  Serial.begin(SERIAL_BAUD);
  while(!Serial)   // Wait for serial port initialization
  {
    wdt_reset();
  }
}
#endif

void setupWiFi()
{
  mlog(S_INFO, "Start initialization");
  mlog(S_DEBUG, "WiFi SSID: " + String(WIFI_SSID) + " WiFi password: " + String(WIFI_PASSWORD));
  
  wifi_set_sleep_type(LIGHT_SLEEP_T); // LIGHT_SLEEP_T for sleep, NONE_SLEEP_T FOR NO SLEEP
  
  WiFi.hostname(HOST_NAME);
  WiFi.mode(WIFI_STA);
  
  mlog(S_INFO, "WiFi MAC: " + WiFi.macAddress());
  mlog(S_INFO, "Connecting to " + String(WIFI_SSID));

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    mlog(S_INFO, "Waiting for WiFi connection");
    delay(1000);
    wdt_reset();
  }

  mlog(S_INFO, "WiFi connected to " + String(WIFI_SSID));

  IPAddress localIpAddress = WiFi.localIP();
  char ipc[26];
  sprintf(ipc, "%d.%d.%d.%d", localIpAddress[0], localIpAddress[1], localIpAddress[2], localIpAddress[3]);
  mlog(S_INFO, "IP address: " + String(ipc));
}

void setupNtp()
{
  // NTP setup
  Udp.begin(LOCAL_PORT);
  setSyncProvider(getNtpTime);
  setSyncInterval(NTP_UPDATE);
}

void setupSensor()
{
  // Sensor start
  Wire.begin();
  
  while (!mySensor.begin())
  {
    mlog(S_ERROR, "Could not find BME280 sensor!");
    
    delay(1000);
    wdt_reset();
  }

  switch (mySensor.chipModel())
  {
     case BME280::ChipModel_BME280:
     {
       mlog(S_DEBUG, "Found BME280 sensor! Success.");
       break;
     }
     case BME280::ChipModel_BMP280:
     {
       mlog(S_WARNING, "Found BMP280 sensor! No Humidity available.");
       break;
     }
     default:
     {
       mlog(S_ERROR, "Found UNKNOWN sensor! Error!");
     }
  }
}

#ifdef OTA
void setupOta()
{
  // OTA update

  // Port defaults to 8266
  ArduinoOTA.setPort(OTA_PORT);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(HOST_NAME);
  
  // No authentication by default
  ArduinoOTA.setPassword(OTA_PASSWORD);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  
  ArduinoOTA.onStart([]()
  {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    {
      // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    mlog(S_INFO, "OTA Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]()
  {
    mlog(S_INFO, "OTA end");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
  {
    mlog(S_INFO, "OTA update progress: " + String((progress / (total / 100))));

    // make the LED blinking
    if (progress % 2 == 0)
    {
      digitalWrite(LED_BUILTIN, HIGH);  // turn led off
    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW);  // turn led on
    }
    
  });
  
  ArduinoOTA.onError([](ota_error_t error)
  {
   mlog(S_ERROR, "OTA error[" + String(error) + "]:");
    
    if (error == OTA_AUTH_ERROR)
    {
      mlog(S_ERROR, "OTA Auth Failed");
    }
    else
    if (error == OTA_BEGIN_ERROR)
    {
      mlog(S_ERROR, "OTA Begin Failed");
    }
    else
    if (error == OTA_CONNECT_ERROR)
    {
      mlog(S_ERROR, "OTA Connect Failed");
    }
    else
    if (error == OTA_RECEIVE_ERROR)
    {
      mlog(S_ERROR, "OTA Receive Failed");
    }
    else
    if (error == OTA_END_ERROR)
    {
      mlog(S_ERROR, "OTA End Failed");
    }
  });
  
  ArduinoOTA.begin();
  mlog(S_INFO, "OTA ready");
}
#endif

void loop()
{
  mlog(S_INFO, "Current firmware version: " + String(FW_VERSION));
  
  #ifdef OTA
    ArduinoOTA.handle();
  #endif
  
  wdt_reset();
  
  digitalWrite(LED_BUILTIN, LOW); // turn led on to show we are working
  #ifndef DEEP_SLEEP
    loopstart = millis();
  #endif
  
  if (justRunOnce == true)
  {
    justRunOnce = false;
    // Code that should just run once after start comes here
    // This is to avoid WDT issues in the setup
    delay(10); // sensor needs about 2ms to start up
  }
  
  // Read sensor data into the buffer
  if (now() >= 100000000) // Make sure we received the time via NTP, no use reading sensor data without proper timestamp
  {
    for (int i = 0; i <= BUFFER_SIZE ; i++)
    {
      wdt_reset();
      
      mlog(S_DEBUG, "i = " + String(i) + " BUFFER_SIZE = " + String(BUFFER_SIZE) + " bufferposition = " + String(bufferposition));
      
      if (timeBuffer[bufferposition] == 0)
      {
        break; // Exit the for-loop when we are at an empty buffer position
      }
      else
      {
        bufferposition = (bufferposition + 1) % BUFFER_SIZE; // Try the next position. Automagically rolls over if larger than the buffer
     }
    }
  
    timeBuffer[bufferposition] = now();
  
    float temp(NAN), hum(NAN), pres(NAN);
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);
    
    mySensor.read(pres, temp, hum, tempUnit, presUnit);   
    
    pressureBuffer[bufferposition] = pres;
    tempBuffer[bufferposition] = temp;
    humidityBuffer[bufferposition] = hum;    
    voltageBuffer[bufferposition] = analogRead(A0) * VOLTAGE_MULTIPLIER; 
  
    mlog(S_INFO, "Pressure: " + String(pressureBuffer[bufferposition]) + " Temperature: " + String(tempBuffer[bufferposition]) + " Humiditiy: " + String(humidityBuffer[bufferposition]) + " Voltage: " + voltageBuffer[bufferposition]);
    mlog(S_INFO, String(bufferposition) + " data point are waiting for sending");
  }
  
  if (WiFi.status() == WL_CONNECTED)
  {
    httpUpdate();
    
    if (timeStatus() != timeSet || now() <= 100000000)
    {
      getNtpTime(); // Update NTP time when necessary
    }
    else
    {
      if (sendSensorData("0")) // Test if we can connect to the php page and send data if we can
      {
        mlog(S_DEBUG, "Connection test is okay, send data!");
        
        sendSensorData(sensorData()); // sending up to 30 datasets at once (we can only send about 3000 characters in the POST request)
        
        mlog(S_INFO, "All data sent.");
      }
      else
      {
        mlog(S_ERROR, "Failed to connect");
      }
    }
  }
  
  // Go to sleep for the specified time (minus the time we needed for all the stuff we did in the loop)
  loopend = millis();
  worktime = loopend - loopstart;
  realdelay = calculateDelayTime(analogRead(A0) * VOLTAGE_MULTIPLIER, SLEEP_DELAY) - worktime;

  if (realdelay > SLEEP_DELAY)
  {
    mlog(S_WARNING, "Real delay was adjusted. It was too big! (" + String(realdelay) + ")");
    
    realdelay = SLEEP_DELAY;
  }
  
  mlog(S_DEBUG, "Work time: " + String(worktime) + " Real delay: " + String(realdelay));
  
  digitalWrite(LED_BUILTIN, HIGH);  // turn led off to show we are sleeping

  // Sleep until the next measurement
  
  mlog(S_INFO, "Sleep for " + String(realdelay / 1000.0) + " sec");

  #ifdef DEEP_SLEEP
    // Clear UART FIFO to enter into deep sleep mode immediatelly without waiting for sending over what is in the FIFO
    //SET_PERI_REG_MASK(UART_CONF0(0), UART_TXFIFO_RST); //RESET FIFO 
    //CLEAR_PERI_REG_MASK(UART_CONF0(0), UART_TXFIFO_RST);
    // https://www.espressif.com/sites/default/files/9b-esp8266-low_power_solutions_en_0.pdf
    ESP.deepSleep(realdelay * 1000);
  #else
    delay(realdelay);
  #endif
  
  // It seems this line never will be called (reboot happens after deep sleep)
  mlog(S_INFO, "Next measurement comes NOW!");
}
