
String SensorData () {
  int i = 0;
  int j = 0;
  String textToSend ="";
  while ( i <= buffersize && j < maxsendsize) {
    if (timeBuffer[i] > 0) {
      if (textToSend != "") textToSend+= "&";
          textToSend += "d[" + String(i) + "][ti]=" + timeBuffer[i] + "&d[" + String(i) + "][t]=" + tempBuffer[i] + "&d[" + String(i) + "][p]=" + pressureBuffer[i] + "&d[" + String(i) + "][h]=" + humidityBuffer[i] + "&d[" + String(i) + "][v]=" + voltageBuffer[i];   
          j++;
          timeBuffer[i] = 0;
          tempBuffer[i] = 0.0;
          pressureBuffer[i] = 0.0;
          humidityBuffer[i] = 0.0; 
          voltageBuffer[i] = 0.0;
          Serial.println(i);       
          }
  i++;
  }
  return textToSend;
}

int SendSensorData (String Sensordata) {
  // returns negative value if unsuccessful
  // returns a value from the php page if successful
  String returnvalue = "-1";
  if (Sensordata != "" ) {
    HTTPClient http;
    http.begin(phppage);
    http.addHeader("Content-Type", contenttype);
    int httpCode = http.POST(Sensordata);
    if (httpCode != 200) {
      returnvalue = "-1";
      Serial.println("not successful");
      } else {
      returnvalue = http.getString();
      Serial.println("successful:" + returnvalue);   
      }
    http.end();
  } 
  int returnval = returnvalue.toInt();
  return returnval;
}


/* not working yet
void fpm_wakup_cb_func1(void) { 
wifi_fpm_close();
wifi_set_opmode(STATION_MODE);
wifi_station_connect();
}
*/




/*-------- NTP code ----------*/

WiFiUDP Udp;
time_t getNtpTime();
void sendNTPpacket(IPAddress &address);


const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
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

