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

    #ifdef SERIAL
      Serial.flush();
    #endif
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
    
    WiFiClient wifiClient;
    HTTPClient post;
    post.useHTTP10(true);
    post.setTimeout(1000);
    post.begin(wifiClient, DATA_REST_ENDPOINT);
    post.addHeader("Content-Type", CONTENT_TYPE);
    int httpCode = post.POST(sensorData);
    String payload = post.getString();
    post.end();
    
    if (httpCode != HTTP_CODE_OK)
    {
      returnValue = false;
      mlog(S_ERROR, "HTTP send failed. Response code: "  + String(httpCode));
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

unsigned long calculateDelayTime(float voltage, unsigned long baseDelay)
{
  /*
   * This function checks current battery voltage and increases delay time lineary based on the measurement.
   * If current voltage is 2.5 V then delay time will be 5 times higher then in case of TARGET_BATTER_VOLTAGE.
   * 2.5 V is the minimum operating voltage of ESP8266.
   * Calculated delay time cannot be less then baseDelay.
   */
  
  #ifdef BATTERY_SAVER_DELAYS
    float factor = 1.0f;
    
    if (voltage >= TARGET_BATTERY_VOLTAGE)
    {
      // factor cannot be smaller then 1
      factor = 1.0f;
    }
    else
    {
      factor = ( ((4.0f*voltage) - (4.0f*TARGET_BATTERY_VOLTAGE)) / (2.5f - TARGET_BATTERY_VOLTAGE) ) + 1;
    }

    // Validations...
    if (factor >= 1)
    {
      // might be good but let's see!
      
      if (factor > 20)
      {
        // too big value!
        factor = 1.0f;
        mlog(S_ERROR, "Factor is too big! Please check it immediatelly! New factor value is 1.0!");
      }
    }
    else
    {
      // too small value!
      factor = 1.0f;  // factor cannot be smaler then 1
      mlog(S_ERROR, "Factor is too small! Please check it immediatelly! New factor value is 1.0!");
    }
    
    unsigned long delayTime = (int)(factor * baseDelay);

    if (delayTime > ESP.deepSleepMax())
    {
      mlog(S_WARNING, "delayTime is bigger then deepSleepMax()! Set to max!");
      delayTime = ESP.deepSleepMax();
    }
    
    mlog(S_DEBUG, "Delay calculator. Batt.: " + String(voltage) + "V Target: " + String(TARGET_BATTERY_VOLTAGE) + "V Calculated factor: " + String(factor) + "x Calculated delay: " + String(delayTime) + "ms");

    return delayTime;
  #else
    mlog(S_DEBUG, "Battery saver mode is disabled!");
    return baseDelay;
  #endif
  
}
