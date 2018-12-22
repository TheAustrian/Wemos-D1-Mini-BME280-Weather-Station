void setupSensor()
{
  // Sensor start
  Wire.begin();
  
  while (!mySensor.begin())
  {
    mlog(S_ERROR, "Could not find BME280 sensor!");
    
    delay(1000);
    wdt_reset();
  }

  switch (mySensor.chipModel())
  {
     case BME280::ChipModel_BME280:
     {
       mlog(S_DEBUG, "Found BME280 sensor! Success.");
       break;
     }
     case BME280::ChipModel_BMP280:
     {
       mlog(S_WARNING, "Found BMP280 sensor! No Humidity available.");
       break;
     }
     default:
     {
       mlog(S_ERROR, "Found UNKNOWN sensor! Error!");
     }
  }
}
