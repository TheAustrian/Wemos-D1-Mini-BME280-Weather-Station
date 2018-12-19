const unsigned long FW_VERSION = 1007;

//#define SERIAL true
#define SERIAL_BAUD 115200

// if DEEP_SLEEP is defined, ESP will be sent to deep sleep state between two measurement
#define DEEP_SLEEP true
// if BATTERY_SAVER_DELAYS defined, delay between two measurements is based on the current battery voltage.
#define BATTERY_SAVER_DELAYS true

// Sleep time is varied based on the power voltage to ensure battery is always chared properly
const float TARGET_BATTERY_VOLTAGE = 3.8; // 3.7 V is the voltage of my battery, but we measure a bit over so let's use a bit higher target here

// Network settings
const char* HOST_NAME = "WeMosMini";
const char* WIFI_SSID = "MyWifiSsid";
const char* WIFI_PASSWORD = "MySuperSecretWiFiPAssword123";

const char* DATA_REST_ENDPOINT = "http://www.example.com/espdata.php";
const char* CONTENT_TYPE = "application/x-www-form-urlencoded";

// OTA update parameters
//#define OTA true
#ifdef OTA
  unsigned int OTA_PORT = 8266;
  const char* OTA_PASSWORD = "123";
#endif

#define HTTP_UPDATE true
#ifdef HTTP_UPDATE
  const char* HTTP_UPDATE_URL = "http://www.example.com/update.php";
  unsigned long HTTP_UPDATE_INTERVAL = 5;
#endif

// Time settings
#define NTP_UPDATE 1800 // update NTP every 30 mins (in seconds)
static const char ntpServerName[] = "hu.pool.ntp.org"; // use "us.pool.ntp.org" for the US, and other values for other countries analogously 
const int TIME_ZONE = 0;     // Central European Time 1 , CEST 2, but should be handled by the JavaScript when the data is displayed in the browser
unsigned int LOCAL_PORT = 8888;   // local port to listen for UDP packets

// Time between two measurement
const unsigned long SLEEP_DELAY = 60 * 1000; // Time between samples in milliseconds

const float VOLTAGE_MULTIPLIER = 0.00496f; // Using a 200k resistor between the battery voltage and analog input A0 this experimentally results in the correct voltage being calculated in my setup, this might need to be changed for yours

// Number of locally stored data point if remote server is unavailable
const unsigned int BUFFER_SIZE = 60 * 12; // 12 hours buffer when taking 1 sample per minute

// Number of data point what is sent to the remote server in one single REST call
const unsigned int MAX_SEND_SIZE = 50; // Maximum number of datasets sent per POST request. If the request gets too large, stuff gets cut off, 30 is a safe value

// LOG SEVERITY
const int S_TRACE = 0;
const int S_DEBUG = 1;
const int S_ERROR = 2;
const int S_WARNING = 3;
const int S_INFO = 4;
const int MIN_LOG_SEVERITY = S_INFO;

BME280I2C mySensor; // This can be tweaked, refer to https://github.com/finitespace/BME280 for details
