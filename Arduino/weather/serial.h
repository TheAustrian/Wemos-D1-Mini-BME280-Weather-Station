#ifdef SERIAL
void setupSerial()
{
  Serial.begin(SERIAL_BAUD);
  Serial.setTimeout(2000);
  while(!Serial)   // Wait for serial port initialization
  {
    wdt_reset();
  }
}
#endif
