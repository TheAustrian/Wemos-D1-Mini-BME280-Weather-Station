void setupWiFi()
{
  mlog(S_INFO, "Start initialization");
  mlog(S_DEBUG, "WiFi 1st SSID: " + String(WIFI_1_SSID) + " WiFi password: " + String(WIFI_1_PASSWORD));
  mlog(S_DEBUG, "WiFi 2nd SSID: " + String(WIFI_2_SSID) + " WiFi password: " + String(WIFI_2_PASSWORD));
  
  //wifi_set_sleep_type(LIGHT_SLEEP_T); // LIGHT_SLEEP_T for sleep, NONE_SLEEP_T FOR NO SLEEP
    
  // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOST_NAME);
  wifiMulti.addAP(WIFI_1_SSID, WIFI_1_PASSWORD);
  wifiMulti.addAP(WIFI_2_SSID, WIFI_2_PASSWORD);
  
  mlog(S_INFO, "WiFi MAC: " + WiFi.macAddress());
  mlog(S_INFO, "Connecting to WiFi network");

  while (wifiMulti.run() != WL_CONNECTED)
  {
    mlog(S_INFO, "Waiting for WiFi connection. Connecting to " + WiFi.SSID());
    delay(1000);
    wdt_reset();
  }

  mlog(S_INFO, "WiFi connected to " + WiFi.SSID());

  IPAddress localIpAddress = WiFi.localIP();
  char ipc[26];
  sprintf(ipc, "%d.%d.%d.%d", localIpAddress[0], localIpAddress[1], localIpAddress[2], localIpAddress[3]);
  mlog(S_INFO, "IP address: " + String(ipc));
}
