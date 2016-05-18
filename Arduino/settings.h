// Network settings
const char* ssid = "YOUR SSID";
const char* password = "YOUR PASSWORD";
const char* espname = "D1-Weather";
const char* phppage = "http://YOURSERVER.com/YOURFOLDER/espdata.php";
const char* contenttype = "application/x-www-form-urlencoded";

// Time settings
static const char ntpServerName[] = "at.pool.ntp.org"; // use "us.pool.ntp.org" for the US, and other values for other countries analogously 
const int timeZone = 0;     // Central European Time 1 , CEST 2, but should be handled by the JavaScript when the data is displayed in the browser
unsigned int localPort = 8888;   // local port to listen for UDP packets

// Application settings
const unsigned long sleepdelay = 60000; // Time between samples in milliseconds

const float voltagemultiplier = 0.00496f; // Using a 200k resistor between the battery voltage and analog input A0 this experimentally results in the correct voltage being calculated in my setup, this might need to be changed for yours

const int buffersize = 60*12; // 12 hours buffer when taking 1 sample per minute
const int maxsendsize = 30; // Maximum number of datasets sent per POST request. If the request gets too large, stuff gets cut off, 30 is a safe value

BME280 mySensor; // This can be tweaked, refer to https://github.com/finitespace/BME280 for details
