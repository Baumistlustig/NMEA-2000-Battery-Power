# NMEA 2000 BUS Voltage

This is a project, which reads the voltage and current of a battery, and posts it to a NMEA 2000 BUS.

The project uses a D1 Mini (ESP8266) and a MCP2515 Module for CAN BUS communication. If the voltage falls under a certain value, a relay will automatically disconnect the battery from the rest of the circuit to prevent drain. 

Additionally, it posts the voltage into the N2k bus where it can be worked with by other devices.
