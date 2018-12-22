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
