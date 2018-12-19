void mlog(const int severity, StringSumHelper message)
{
  if (severity >= MIN_LOG_SEVERITY)
  {  
    switch (severity)
    {
      case S_TRACE:
      {
        #ifdef SERIAL
          Serial.println("TRACE - " + message);
        #endif
        
        break;
      }
      
      case S_DEBUG:
      {
        #ifdef SERIAL
          Serial.println("DEBUG - " + message);
        #endif
        
        break;
      }
  
      case S_INFO:
      {
        #ifdef SERIAL
          Serial.println("INFO - " + message);
        #endif
        
        break;
      }
  
      case S_WARNING:
      {
        #ifdef SERIAL
          Serial.println("WARNING - " + message);
        #endif
        
        break;
      }
  
      case S_ERROR:
      {
        #ifdef SERIAL
          Serial.println("ERROR - " + message);
        #endif
        
        break;
      }
    }
  }
  else
  {
    // do nothing, log message needs to be droped  
  }
}

String sensorData()
{
  int i = 0;  // Buffer index variable
  int j = 0;  // Counter of converted data records
  String textToSend = "";
  
  mlog(S_DEBUG, "Buffer size: " + String(BUFFER_SIZE) + " Max send size: " + String(MAX_SEND_SIZE));
  
  while (i < BUFFER_SIZE && j < MAX_SEND_SIZE)
  {
    if (timeBuffer[i] > 0)
    {
      if (textToSend != "")
      {
          textToSend = textToSend + "&";
      }

      // convert current buffer position to the sending format
      textToSend += "d[" + String(i) + "][id]=" + String(WiFi.macAddress()) + "&d[" + String(i) + "][ti]=" + timeBuffer[i] + "&d[" + String(i) + "][t]=" + tempBuffer[i] + "&d[" + String(i) + "][p]=" + pressureBuffer[i] + "&d[" + String(i) + "][h]=" + humidityBuffer[i] + "&d[" + String(i) + "][v]=" + voltageBuffer[i];   

      // Note that i. buffer possition is already converted
      timeBuffer[i] = 0;
      tempBuffer[i] = 0.0;
      pressureBuffer[i] = 0.0;
      humidityBuffer[i] = 0.0; 
      voltageBuffer[i] = 0.0;

      mlog(S_TRACE, textToSend);
      
      wdt_reset();
      
      j++;
    }
    
    i++;
  }

  mlog(S_DEBUG, "textToSend = " + textToSend);
  
  return textToSend;
}

bool sendSensorData(String sensorData)
{
  bool returnValue = false;
  
  if (sensorData != "")
  {
    wdt_reset();
    
    HTTPClient post;
    post.begin(DATA_REST_ENDPOINT);
    post.addHeader("Content-Type", CONTENT_TYPE);
    int httpCode = post.POST(sensorData);
    String payload = post.getString();
    post.end();
    
    if (httpCode != 200)
    {
      returnValue = false;
      
      mlog(S_INFO, "HTTP send failed. Http code: " + String(httpCode));
    }
    else
    {
      returnValue = true;
      
      mlog(S_INFO, "HTTP send okay. Response payload:" + payload);
    }
  }
  else
  {
    mlog(S_ERROR, "HTTP POST data is empty but it should not be!");
  }
  
  return returnValue;
}

/*-------- NTP code ----------*/

WiFiUDP Udp;
time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  wdt_reset();
  
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) // discard any previously received packets
  {
    wdt_reset();
  }

  mlog(S_DEBUG, "Transmit NTP Request");
  
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);

  char ipc[26];
  sprintf(ipc, "%d.%d.%d.%d", ntpServerIP[0], ntpServerIP[1], ntpServerIP[2], ntpServerIP[3]);
  mlog(S_DEBUG, String(ntpServerName) + " has " + String(ipc) + " IP");
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500)
  {
    wdt_reset();
    
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE)
    {
      mlog(S_INFO, "Receive NTP response");
      
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + TIME_ZONE * SECS_PER_HOUR;
    }
  }

  mlog(S_ERROR, "No NTP response");
  
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  wdt_reset();
  
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

/*-------- HTTP update code ----------*/
void httpUpdate()
{
  #ifdef HTTP_UPDATE
    wdt_reset();
    
    int i = minute(now()) % HTTP_UPDATE_INTERVAL;
    if (i != 0)
    {
      mlog(S_INFO, "HTTP update will happen in " + String(HTTP_UPDATE_INTERVAL-i) + " minutes");
      return;
    }

    wdt_reset();
    
    // The line below is optional. It can be used to blink the LED on the board during flashing
    // The LED will be on during download of one buffer of data from the network. The LED will
    // be off during writing that buffer to flash
    // On a good connection the LED should flash regularly. On a bad connection the LED will be
    // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
    // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    wdt_reset();
    
    WiFiClient client;
    String fwUrl = String(HTTP_UPDATE_URL);
    fwUrl.concat("?version=");
    fwUrl.concat(String(FW_VERSION));
    fwUrl.concat("&mac=");
    fwUrl.concat(WiFi.macAddress());
    mlog(S_DEBUG, "Firmware update URL: " + fwUrl);
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, fwUrl);
    // Or:
    //t_httpUpdate_return ret = ESPhttpUpdate.update(client, "server", 80, "file.bin");

    wdt_reset();
    
    switch (ret)
    {
      case HTTP_UPDATE_FAILED:
      {
        mlog(S_ERROR, "HTTP_UPDATE_FAILD Error (" + String(ESPhttpUpdate.getLastError()) + "): " + ESPhttpUpdate.getLastErrorString());
        break;
      }
      case HTTP_UPDATE_NO_UPDATES:
      {
        mlog(S_INFO, "HTTP_UPDATE_NO_UPDATES");
        break;
      }
      case HTTP_UPDATE_OK:
      {
        mlog(S_INFO, "HTTP_UPDATE_OK");
        break;
      }
    }
  #else
    mlog(S_DEBUG, "HTTP update feature was excluded at build time");
  #endif
}

unsigned long calculateDelayTime(float voltage, unsigned long baseDelay)
{
  /*
   * This function checks current battery voltage and increases delay time lineary based on the measurement.
   * If current voltage is 2.5 V then delay time will be 5 times higher then in case of TARGET_BATTER_VOLTAGE.
   * 2.5 V is the minimum operating voltage of ESP8266.
   * Calculated delay time cannot be less then baseDelay.
   */
  
  #ifdef BATTERY_SAVER_DELAYS
    unsigned long factor = ( (4*voltage - 4*TARGET_BATTERY_VOLTAGE) / (2,5 - TARGET_BATTERY_VOLTAGE) ) + 1;
    factor = factor >= 1 ? factor : 1.0;  // factor cannot be smaler then 1
    
    unsigned long delayTime = factor * baseDelay;
    
    mlog(S_DEBUG, "Delay calculator. Batt.: " + String(voltage) + "V Target: " + String(TARGET_BATTERY_VOLTAGE) + "V Calculated factor: " + String(factor) + " Calculated delay: " + String(delayTime));

    return delayTime;
  #else
    mlog(S_DEBUG, "Battery saver mode is disabled!");
    return baseDelay;
  #endif
  
}
