# Wemos D1 Mini - BME280 Weather Station
Simple Weather Station Code using an ESP8266 and a BME280 sensor.

## Features
- Optimized for baterry usage
- Buffer for when there is no Internet connection available. Data is sent when network came back.
- NTP time syncronization
- Over-the-Air update from HTTP server with PHP script capability
- Collect measured data with remote PHP script
- Logging for serial connection
- LED is used to show ongoing measurement

## Parts List:
- 1x Wemos D1 Mini ESP8266 development board
- 1x BME280 breakout board
- 1x TP4056 charging board with battery protection
- 2x Solar panels 5V 240mA (or one bigger one)
- 1x Battery holder for 18650 batteries
- 1x 18650 lithium ion battery
- 2x Schottky diodes
- 1x 200k Ohm resistor

## Schematic:
![](schematic.jpg)
