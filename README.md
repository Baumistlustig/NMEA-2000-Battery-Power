# NMEA 2000 Battery Power

This is a project, which reads the voltage and current of a battery, and posts it to a NMEA 2000 BUS.

The project uses an ESP32 & the  for CAN BUS communication. If the voltage falls under a certain value, a contactor will automatically disconnect the battery from the rest of the circuit to prevent drain. 

Additionally, it posts the voltage, current and temperature into the N2k bus where it can be worked with by other devices.

## Table of Contents

- [Electrical](/Electrical) A KiCad project 
- [Software](/Software) A PlatformIO project
