# NMEA 2000 BUS Voltage

This is a project, which reads the voltage of a battery, and posts it to a NMEA 2000 BUS

The project uses a ESP32 and a 65HVD230 Module for CAN BUS communication. If the voltage falls under a certain value, a relay will automatically disconnect the battery from the rest of the circuit to prevent drain. 
