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

/* 
 * Update from Arduino IDE
 */
#ifdef OTA
void setupOta()
{
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
