#include <buffer.h>
#include <crc.h>
#include <datatypes.h>
#include <local_datatypes.h>
#include <printf.h>
#include <VescUart.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

#define SERIALIO Serial
#define SCHEMA 0x0101



LiquidCrystal lcd(2, 4, 7, 8, 12, A0);
int page = 0;
int input;
int maxSpeed = 0;
float totalKm = 0;
float totalChargeKm = 0;
float totalAh = 0;

unsigned long stoppedTime = 0;
unsigned long rollingTime = 0;
unsigned long lastStop;
unsigned long startTime;

struct bldcMeasure measuredValues;
SoftwareSerial sensitiveLink(A1, A2);
/* Values:
    avgMotorCurrent
    avgInputCurrent
    dutyCycleNow
    rpm
    inpVoltage
    ampHours
    ampHoursCharged
    tachometer
    tachometerAbs
*/
template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}


template <class T> int EEPROM_readAnything(int ee, T& value)
{
  byte* p = (byte*)(void*)&value;
  int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}


void setup() {
  sensitiveLink.begin(9600);
  SERIALIO.begin(9600); //Vesc serial (Serial)
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Voltage: ");
  lcd.setCursor(0, 1);
  lcd.print("Ah drawn: ");
  lastStop = millis();
  startTime = millis();
  EEPROM_readAnything( sizeof(long) + 1, totalKm);
  EEPROM_readAnything( sizeof(long) + 10, totalAh);
  EEPROM_readAnything( sizeof(long) + 20, totalChargeKm);
}

void loop() {
  delay(5);
  EEPROM_writeAnything( sizeof(long) + 1, (totalKm + (float)((measuredValues.tachometer / 44) * 15 / 36 * 3.1415926536 * 0.000083) ));
  EEPROM_writeAnything( sizeof(long) + 10, (totalAh + (float)(measuredValues.ampHours)));
  EEPROM_writeAnything( sizeof(long) + 20, (totalChargeKm + (float)((measuredValues.tachometer / 44) * 15 / 36 * 3.1415926536 * 0.000083) ));
  if (VescUartGetValue(measuredValues)) {
    if (measuredValues.inpVoltage > 41) {
      totalAh = 0;
      totalChargeKm = 0;
    }

    if (measuredValues.rpm * 60 * 15 * 3.1415926536 * 0.000083 / 36 / 7 > maxSpeed) {
      maxSpeed = measuredValues.rpm * 60 * 15 * 3.1415926536 * 0.000083 / 36 / 7;
    }
    if (measuredValues.rpm * 60 * 15 * 3.1415926536 * 0.000083 / 36 / 7 <= 2) {
      stoppedTime += 5;
    }
    rollingTime = millis() - startTime;

    unsigned long timeElapsed = rollingTime / 1000 / 60;
    int rollTime = int(timeElapsed);

    switch (page) {
      case 0:
        lcd.setCursor(0, 0);
        lcd.print("Voltage: ");
        lcd.print(measuredValues.inpVoltage);
        lcd.print("V        ");
        lcd.setCursor(0, 1);
        lcd.print("Cycle: ");
        lcd.print(totalChargeKm + (float)((measuredValues.tachometer / 44) * 15 / 36 * 3.1415926536 * 0.000083));
        lcd.print("km        ");
        break;
      case 1:
        lcd.setCursor(0, 0);
        lcd.print("Total: ");
        lcd.print((totalKm + (float)((measuredValues.tachometer / 44) * 15 / 36 * 3.1415926536 * 0.000083) ));
        lcd.print("km        ");
        lcd.setCursor(0, 1);
        lcd.print("Dist: ");
        lcd.print((measuredValues.tachometer / 44) * 15 / 36 * 3.1415926536 * 0.000083);
        lcd.print("km        ");
        break;
      case 2:
        lcd.setCursor(0, 0);
        lcd.print("Ah drawn: ");
        lcd.print(measuredValues.ampHours);
        lcd.print("Ah        ");
        lcd.setCursor(0, 1);
        lcd.print("Ah regen: ");
        lcd.print(measuredValues.ampHoursCharged);
        lcd.print("Ah        ");
        break;
      case 3:
        lcd.setCursor(0, 0);
        lcd.print("Max Speed: ");
        lcd.print(maxSpeed);
        lcd.print("kph        ");
        lcd.setCursor(0, 1);
        lcd.print("Time: ");
        lcd.print(rollTime);
        lcd.print("min        ");
        break;
      case 4:
        lcd.setCursor(0, 0);
        lcd.print("Ride Wh: ");
        lcd.print((measuredValues.ampHours - measuredValues.ampHoursCharged) * measuredValues.inpVoltage);
        lcd.print("Wh          ");
        lcd.setCursor(0, 1);
        lcd.print("Avg: ");
        lcd.print(((measuredValues.ampHours - measuredValues.ampHoursCharged) * measuredValues.inpVoltage) / ((measuredValues.tachometer / 44) * 15 / 36 * 3.1415926536 * 0.000083));
        lcd.print("Wh/km       ");
        break;
      case 5:
        lcd.setCursor(0, 0);
        lcd.print("Cycle Wh: ");
        lcd.print((totalAh + measuredValues.ampHours) * 37);
        lcd.print("Wh          ");
        lcd.setCursor(0, 1);
        lcd.print("Avg: ");
        lcd.print((totalAh + measuredValues.ampHours) * 37 / (totalChargeKm + (float)((measuredValues.tachometer / 44) * 15 / 36 * 3.1415926536 * 0.000083) ));
        lcd.print("Wh/km          ");
        break;
    }
  }
  input = 0;
  if (sensitiveLink.available()) {
    while (sensitiveLink.available()) {
      input = sensitiveLink.read();
    }
    if (input == 2) {
      page++;
      if (page == 6) {
        page = 0;
      }
    }
    if (input == 1) {
      page --;
      if (page == -1) {
        page = 5;
      }
    }
  }
}