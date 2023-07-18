#include <Arduino.h>
#include <Preferences.h>
#include <NMEA2000_CAN.h>
#include <N2kMessages.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#define ESP32_CAN_TX_PIN GPIO_NUM_5  // Set CAN TX port to 5 
#define ESP32_CAN_RX_PIN GPIO_NUM_4  // Set CAN RX port to 4

#define ADCpin1 34                   // GPIO 34 (Analog input, ADC1_CH6)
#define ADC_Calibration_Value1 250.0 // For resistor measure 5 Volt and 180 Ohm equals 100% plus 1K resistor.

#define ADCpin2 35                   // GPIO 35 (Analog input, ADC1_CH7))
#define ADC_Calibration_Value2 34.3  // To measure battery voltage. The real value depends on the true resistor values for the ADC input (100K / 27 K).


// Set time offsets
#define SlowDataUpdatePeriod 1000  // Time between CAN Messages sent
#define BatteryVoltageSendOffset 0
#define WaterTankLevelSendOffset 50

Adafruit_BMP280 bmp;

int NodeAddress;            // To store last Node Address
Preferences preferences; 

const unsigned long TransmitMessages[] PROGMEM = {
  127508L, // Battery Status
  127505L, // Fluid level
  0
};

// Define all Function Codes
void CheckSourceAddressChange();
void sendBatteryInfo(void);
bool IsTimeToUpdate(unsigned long NextUpdate);
unsigned long InitNextUpdate(unsigned long Period, unsigned long Offset = 0);
void SetNextUpdate(unsigned long & NextUpdate, unsigned long Period);
double ReadADC(byte pin);

void setup() {
  
  uint8_t chipid[6];
  uint32_t id = 0;
  int i = 0;

  // Init USB serial port
  Serial.begin(115200);
  delay(10);

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);

    bmp.setSampling(
      Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
      Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
      Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
      Adafruit_BMP280::FILTER_X16,      /* Filtering. */
      Adafruit_BMP280::STANDBY_MS_500   /* Standby time. */
    );
  }

  // Reserve enough buffer for sending all messages.
  NMEA2000.SetN2kCANMsgBufSize(8);
  NMEA2000.SetN2kCANReceiveFrameBufSize(150);
  NMEA2000.SetN2kCANSendFrameBufSize(150);

  // Generate unique number from chip id
  esp_efuse_mac_get_default(chipid);
  for (i = 0; i < 6; i++) id += (chipid[i] << (7 * i));

  // Set product information
  NMEA2000.SetProductInformation(
    "1", // Manufacturer's Model serial code
    100, // Manufacturer's product code
    "My Sensor Module",  // Manufacturer's Model ID
    "1.0.2.25 (2019-07-07)",  // Manufacturer's Software version code
    "1.0.2.0 (2019-07-07)" // Manufacturer's Model version
  );

  // Set device information
  NMEA2000.SetDeviceInformation(
    id, // Unique number. Use e.g. Serial number.
    132, // Device function=Analog to NMEA 2000 Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
    25, // Device class=Inter/Intranetwork Device. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
    2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
  );

  preferences.begin("nvs", false);                          // Open nonvolatile storage (nvs)
  NodeAddress = preferences.getInt("LastNodeAddress", 34);  // Read stored last NodeAddress, default 34
  preferences.end();
  Serial.printf("NodeAddress=%d\n", NodeAddress);

  // If you also want to see all traffic on the bus use N2km_ListenAndNode instead of N2km_NodeOnly below
  NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly, NodeAddress);
  NMEA2000.ExtendTransmitMessages(TransmitMessages);

  NMEA2000.Open();

  delay(200);
}

void loop() {

  sendBatteryInfo();
  
  NMEA2000.ParseMessages();

  CheckSourceAddressChange();
  
  // Dummy to empty input buffer to avoid board to stuck with e.g. NMEA Reader
  if ( Serial.available() ) {
    Serial.read();
  }
}

void CheckSourceAddressChange() {
  int SourceAddress = NMEA2000.GetN2kSource();

  if (SourceAddress != NodeAddress) { // Save potentially changed Source Address to NVS memory
    NodeAddress = SourceAddress;      // Set new Node Address (to save only once)
    preferences.begin("nvs", false);
    preferences.putInt("LastNodeAddress", SourceAddress);
    preferences.end();
    Serial.printf("Address Change: New Address=%d\n", SourceAddress);
  }
}

void sendBatteryInfo(void) {
  static unsigned long SlowDataUpdated = InitNextUpdate(SlowDataUpdatePeriod, BatteryVoltageSendOffset);
  tN2kMsg N2kMsg;
  double BatteryVoltage;
  double BatteryCurrent;
  float BatteryTemperature;

  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated, SlowDataUpdatePeriod);

    // Read BatteryVoltage on ADCpin1
    BatteryVoltage = ReadADC(ADCpin1) * ADC_Calibration_Value1 / 4096;

    // Read BatteryCurrent on ADCpin2
    BatteryCurrent = ReadADC(ADCpin2) * ADC_Calibration_Value2 / 4096;

    // Read BatteryTemperature on BMP280
    BatteryTemperature = bmp.readTemperature();

    // Print values
    Serial.printf(
      "Battery: Voltage: %3.1f V, Current %3.1f A, Temperature: %3.1f \n",
      BatteryVoltage, BatteryCurrent, BatteryTemperature
    );

    // Set N2K message
    SetN2kPGN127508(N2kMsg, 0, BatteryVoltage, BatteryCurrent, BatteryTemperature, 0);

    // Send message
    NMEA2000.SendMsg(N2kMsg);
  }
}

bool IsTimeToUpdate(unsigned long NextUpdate) {
  return (NextUpdate < millis());
}

unsigned long InitNextUpdate(unsigned long Period, unsigned long Offset) {
  return millis() + Period + Offset;
}

void SetNextUpdate(unsigned long & NextUpdate, unsigned long Period) {
  while ( NextUpdate < millis() ) NextUpdate += Period;
}

double ReadADC(byte pin) {
  double reading = analogRead(pin); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
  if (reading < 1 || reading > 4095) return 0;
  // return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
  return (-0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089) * 1000;
} // Added an improved polynomial, use either, comment out as required

