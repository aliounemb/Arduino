#include <buffer.h>
#include <crc.h>
#include <datatypes.h>
#include <local_datatypes.h>
#include <printf.h>
#include <VescUart.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

#define SERIALIO Serial


LiquidCrystal lcd(2, 4, 7, 8, 12, A0);

struct bldcMeasure measuredValues;
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

void setup() {
  SERIALIO.begin(9600); //Vesc serial (Serial)
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Voltage: ");
  lcd.setCursor(0, 1);
  lcd.print("Ah drawn: ");
}

void loop() {
  delay(25);
  if (VescUartGetValue(measuredValues)) {
    lcd.setCursor(9,0);
    lcd.print(measuredValues.inpVoltage);
    lcd.print("      ");
    lcd.setCursor(10, 1);
    lcd.print(measuredValues.ampHours);
    lcd.print("      ");
  }
}
