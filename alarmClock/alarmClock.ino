#include "segments.h"
#include "pins.h"
#include "pitches.h"
#include "functions.h"

#define DEBUG 0
#define REFRESHSPEED 2
#define BLINKSPEED 500
#define TIMEOUT 10000

boolean chars[22][7] = {{1, 1, 1, 1, 1, 1, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0}, //1
    {1, 1, 0, 1, 1, 0, 1}, //2
    {1, 1, 1, 1, 0, 0, 1}, //3
    {0, 1, 1, 0, 0, 1, 1}, //4
    {1, 0, 1, 1, 0, 1, 1}, //5
    {1, 0, 1, 1, 1, 1, 1}, //6
    {1, 1, 1, 0, 0, 0, 0}, //7
    {1, 1, 1, 1, 1, 1, 1}, //8
    {1, 1, 1, 1, 0, 1, 1}, //9
    {1, 1, 1, 0, 1, 1, 1}, //A
    {1, 0, 0, 1, 1, 1, 0}, //C
    {1, 0, 0, 1, 1, 1, 1}, //E
    {1, 0, 0, 0, 1, 1, 1}, //F
    {0, 1, 1, 0, 1, 1, 0}, //H
    {0, 0, 0, 0, 1, 1, 0}, //I
    {0, 1, 1, 1, 1, 0, 0}, //J
    {0, 0, 0, 1, 1, 1, 0}, //L
    {0, 0, 1, 0, 1, 0, 1}, //n
    {1, 1, 0, 0, 1, 1, 1}, //P
    {0, 1, 1, 1, 1, 1, 0}, //U
    {0, 0, 0, 0, 0, 0, 0}}; //Off segment

unsigned long flipmillis;
unsigned long blinkmillis;

boolean blinkState = true;
boolean segments[6][8];
boolean blink[6] = {false};
short onSegment = 0;

void setup() {
        Serial.begin(9600); //Init Serial
    //Init segments pins
    for (int i = 0; i < 6; i++) {
        pinMode(cathodeSegments[i], OUTPUT);
        digitalWrite(cathodeSegments[i], HIGH);
    }
    for (int i = 0; i < 8; i++) {
        pinMode(anodeSegments[i], OUTPUT);
        digitalWrite(anodeSegments[i], LOW);
    }
    
    //Play init sound

   //Set time
   setTime(0, 0, 0, 0, 1, 2015);
   setTimeUser();
}

void loop() {
    
}

void displayTime() {
	short array[6];
	if (hour() < 10){
		array[0] = OFFSEG;
	}
	else {
		array[0] = hour()/10;
	}
	array[1] = hour()%10;
	array[2] = minute()/10;
	array[3] = minute()%10;
	array[4] = second()/10;
	array[5] = second()%10;
	display(array);
}

void display(short text[6]) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            segments[i][j] = chars[text[i]][j];
        }
    }
    
}

void setSeparators(boolean separators[6]) {
    for (int i = 0; i < 6; i++) {
        segments[i][7] = separators[i];
    }
}

void flip() {
    if (millis() - flipmillis > REFRESHSPEED) {
        digitalWrite(cathodeSegments[onSegment], LOW);
        onSegment++;
        if (onSegment == 6) {
            onSegment = 0;
        }
	if (millis() - blinkmillis > BLINKSPEED) {
		blinkState = !blinkState;
	}
	if (blink[onSegment]) {
		digitalWrite(cathodeSegments[onSegment], blinkState);
	}
	else {
        	digitalWrite(cathodeSegments[onSegment], HIGH);
	}
        for (int i = 0; i < 8; i++) {
            digitalWrite(anodeSegments[i], segments[onSegment][i]);
        }
        flipmillis = millis();
    }
}

void setTimeUser() {
	unsigned long lastAction = millis();
	boolean done = false;
	boolean edit = false;
	short selection = 0;
	int pastAnalog = analogRead(analogInput);
	while (!done){
		flip();
		for (int i = 0; i < 6; i++){
			if ((i == (2*selection)) || (i== ((2*selection)+1))) {
				blink[i] = true;
			}
			else {
				blink[i] = false;
			}
		}
		if ((analogRead(analogInput) > (pastAnalog + 5)) || (analogRead(analogInput) < (pastAnalog - 5))){
			edit = true;
			lastAction = millis();
		}
		if (edit){
			switch (selection) {
				case 0:
					setTime(map(analogRead(analogInput), 0, 1023, 0, 23), minute(), second(), day(), month(), year());
					break;
				case 1:
					setTime(hour(), map(analogRead(analogInput), 0, 1023, 0, 59), second(), day(), month(), year());
					break;
				case 2:
					setTime(hour(), minute(), map(analogRead(analogInput), 0, 1023, 0, 59), day(), month(), year());
					break;
				default:
					break;
			}
		}
		if (millis() - lastAction > TIMEOUT) {
			done = true;
		}
		if (readCapSensor()){
			selection ++;
			pastAnalog = analogRead(analogInput);
			edit = false;
			lastAction = millis();
			if (selection == 3) {
			done = true;
			}
		}
		if (done) {
			for (int i = 0; i < 6; i++) {
				blink[i] = false;
			}
		}
	}
}

