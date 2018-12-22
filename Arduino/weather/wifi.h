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
