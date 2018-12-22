#ifdef SERIAL
void setupSerial()
{
  Serial.begin(SERIAL_BAUD);
  while(!Serial)   // Wait for serial port initialization
  {
    wdt_reset();
  }
}
#endif
