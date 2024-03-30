/*
 * EBYTE LoRa E220
 * Stay in sleep mode and wait a wake up WOR message
 *
 * You must configure the address with 0 2 23 (FIXED RECEIVER configuration)
 * and pay attention that WOR period must be the same of sender
 *
 * You must uncommend the correct constructor and set the correct AUX_PIN define.
 *
 * by Renzo Mischianti <https://www.mischianti.org>
 *
 * https://www.mischianti.org
 *
 * E220		  ----- WeMos D1 mini	----- esp32			----- Arduino Nano 33 IoT	----- Arduino MKR	----- stm32               ----- ArduinoUNO
 * M0         ----- D7 (or GND)		----- 19 (or GND)	----- 4 (or GND)			----- 2 (or GND)	----- PB0 (or GND)        ----- 7 Volt div (or GND)
 * M1         ----- D6 (or 3.3v)	----- 21 (or 3.3v)	----- 6 (or 3.3v)			----- 4 (or 3.3v)	----- PB10 (or 3.3v)      ----- 6 Volt div (or 3.3v)
 * TX         ----- D3 (PullUP)		----- TX2 (PullUP)	----- TX1 (PullUP)			----- 14 (PullUP)	----- PA2 TX2 (PullUP)    ----- 4 (PullUP)
 * RX         ----- D4 (PullUP)		----- RX2 (PullUP)	----- RX1 (PullUP)			----- 13 (PullUP)	----- PA3 RX2 (PullUP)    ----- 5 Volt div (PullUP)
 * AUX        ----- D5 (PullUP)		----- 18  (PullUP)	----- 2  (PullUP)			----- 0  (PullUP)	----- PA0  (PullUP)       ----- 3 (PullUP)
 * VCC        ----- 3.3v/5v			----- 3.3v/5v		----- 3.3v/5v				----- 3.3v/5v		----- 3.3v/5v             ----- 3.3v/5v
 * GND        ----- GND				----- GND			----- GND					----- GND			----- GND                 ----- GND
 *
 */

// With FIXED RECEIVER configuration
//#define DESTINATION_ADDL 2
// With FIXED RECEIVER configuration
#define DES_ADDL	0x00	// controller address
#define DES_ADDH	0x00	// common
#define RF_CHAN		6		// 922MHz (ch.31)

//#define AUX_PIN 3

#include "Arduino.h"
#include "LowPower.h"
#include "LoRa_E220(JP).h"
#include "device_setup.h"

//ResponseContainer rc;
//ResponseStatus rs;
uint8_t waked = 0;

const byte AUX_PIN = 3;
volatile byte state = LOW;

// ---------- Arduino pins --------------
//LoRa_E220 e220ttl(4, 5, 3, 7, 6); // Arduino RX <-- e220 TX, Arduino TX --> e220 RX AUX M0 M1
//LoRa_E220 e220ttl(4, 5); // Config without connect AUX and M0 M1

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(4, 5); // Arduino RX <-- e220 TX, Arduino TX --> e220 RX
//LoRa_E220 e220ttl(&mySerial, 3, 7, 6); // AUX M0 M1
// -------------------------------------

bool interruptExecuted = false;
//uint16_t cntr = 0;

void wakeUp() {
 	// Do not use Serial on interrupt callback
 	interruptExecuted = true;
 	detachInterrupt(digitalPinToInterrupt(AUX_PIN));
}

//The setup function is called once at startup of the sketch
void setup() {

	// initialize digital pin LED_BUILTIN as an output.
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(UserLED, OUTPUT);

	Serial.begin(9600);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB
	}
	digitalWrite(UserLED, HIGH);
	delay(100);

	e220ttl.begin();
	e220ttl.setMode(MODE_2_WOR_RECEIVER);

	delay(100);
	Serial.println();
	Serial.println("Start sleep!");
	delay(100);
    //attachInterrupt(digitalPinToInterrupt(AUX), wakeUp, FALLING);
    attachInterrupt(digitalPinToInterrupt(AUX_PIN), wakeUp, LOW);
	// Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
	digitalWrite(UserLED, LOW);
	waked = 0;
	LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

// The loop function is called in an endless loop
void loop() {

	if (e220ttl.available() > 1)
	{
		Serial.println("Message arrived!");
		ResponseContainer rc = e220ttl.receiveMessageRSSI();
		// First of all get the data
		String message = rc.data;
		Serial.println(rc.status.getResponseDescription());
		String rssi = "RSSI: -"; rssi += String(255 - rc.rssi, DEC); rssi += " dBm";
		Serial.println(rssi);
		Serial.print("Received --> ");
		Serial.println(message);
		if (!waked)
		{
			// Work only with full connection
			e220ttl.setMode(MODE_0_NORMAL);
			waked = 1;
			delay(500);
		}

		if (message.indexOf("pon") >= 0)
		{
			digitalWrite(UserLED, HIGH);
			ResponseStatus rs = e220ttl.sendFixedMessage(DES_ADDH, DES_ADDL, RF_CHAN, "EP received wake-up");
			Serial.println(rs.getResponseDescription());
		}
		else if(message.indexOf("poff") >= 0)
		{
			waked = 0;
			digitalWrite(UserLED, LOW);
			ResponseStatus rs = e220ttl.sendFixedMessage(DES_ADDH, DES_ADDL, RF_CHAN, "EP received goto-sleep");
			Serial.println(rs.getResponseDescription());
		}
		else
		{
			ResponseStatus rs = e220ttl.sendFixedMessage(DES_ADDH, DES_ADDL, RF_CHAN, "EP received a message");
			Serial.println(rs.getResponseDescription());
		}
	}

	if(interruptExecuted) {
		Serial.println("WakeUp Callback, AUX pin go LOW and start receive message!");
		Serial.flush();
		interruptExecuted = false;

		// Work only with full connection
		e220ttl.setMode(MODE_0_NORMAL);
		waked = 1;
		delay(500);
	}
	delay(20);

	if (waked == 0)
	{
		Serial.println();
		Serial.println("Start sleep again!");
		// WOR Receive only
		e220ttl.setMode(MODE_2_WOR_RECEIVER);
		delay(100);
		interruptExecuted = false;
		attachInterrupt(digitalPinToInterrupt(AUX_PIN), wakeUp, LOW);
		// Enter power down state with ADC and BOD module disabled.
	    // Wake up when wake up pin is low.
    	LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
	}
}
