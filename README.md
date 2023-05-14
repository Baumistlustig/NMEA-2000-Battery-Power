# NMEA 2000 BUS Voltage

This is a project, which reads the voltage of a battery, and posts it to a NMEA 2000 BUS

The project uses a Wemos D1 Mini (ESP8266), a 65HVD230 Module for CAN BUS communication. If the voltage falls under a certain value, a relay will automatically disconnect the batter from the rest of the circuit to prevent drain. 

Big thanks to [AK-Homberger](https://github.com/AK-Homberger), he has lots of projects involving the NMEA 2000 BUS and also explains them in a proper way.
