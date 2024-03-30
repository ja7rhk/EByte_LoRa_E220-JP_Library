/*
 * EBYTE LoRa E220
 * Send wake up WOR message to a device in Sleep (WOR transmitter)
 *
  */

// With FIXED SENDER configuration
#define SRC_ADDL	0x00	// unique module address (controller)
#define SRC_ADDH	0x00	// common
#define RF_CHAN		6		// 922MHz (ch.31)
#define BEACON_01 	0x01	// Beacon_01
#define BEACON_02 	0x02	// Beacon_02

#include "Arduino.h"
#include "LoRa_E220(JP).h"
#include "U8g2lib.h"
#include "device_setup.h"

// ----------- OLED 128x32 ----------------------------------
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
// ----------------------------------------------------------

// ---------- Arduino pins --------------
//LoRa_E220 e220ttl(4, 5, 3, 7, 6); // Arduino RX <-- e220 TX, Arduino TX --> e220 RX AUX M0 M1
//LoRa_E220 e220ttl(4, 5); // Config without connect AUX and M0 M1

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(4, 5); // Arduino RX <-- e220 TX, Arduino TX --> e220 RX
//LoRa_E220 e220ttl(&mySerial, 3, 7, 6); // AUX M0 M1
// -------------------------------------

void setup() {

	// initialize digital pin LED_BUILTIN as an output.
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(UserLED, OUTPUT);

	Serial.begin(9600);
	Serial.setTimeout(10000);	// set the maximum milliseconds to wait for serial data.
  	u8g2.begin();
	u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_8x13_tf);
	u8g2.drawStr(0, 10, "E220_Controller");
	u8g2.drawStr(0, 21, "922MHz 13dBm");
	u8g2.sendBuffer();
	delay(500);

	// Startup all pins and UART
	e220ttl.begin();
	e220ttl.setMode(MODE_1_WOR_TRANSMITTER);
	Serial.println("Hi, I'm going to send WOR message!");

	// Send message
	//ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, "Hello, world? WOR!");
	//ResponseStatus rs = e220ttl.sendFixedMessage(0, BEACON_01, RF_CHAN, "Hello, world? WOR!");
	ResponseStatus rs = e220ttl.sendBroadcastFixedMessage(RF_CHAN, "Hello, world? WOR!");
	// Check If there is some problem of succesfully send
	Serial.println(rs.getResponseDescription());
}

uint16_t cntr = 0;
uint16_t fr_cntr = 0;

void loop() {
	// If something available
	if (e220ttl.available() > 1) {
		// read the String message
		ResponseContainer rc = e220ttl.receiveMessage();
		// Is something goes wrong print error
		if (rc.status.code != 1){
			Serial.println(rc.status.getResponseDescription());
	  	}
		else
		{
			// Print the data received
			Serial.println(rc.status.getResponseDescription());
			Serial.println(rc.data);
		}
	}

  	delay(20);
	cntr++;
	if (cntr >= 250)
	{
		cntr = 0;
		fr_cntr++;
		digitalWrite(UserLED, HIGH);
		uint8_t fr = fr_cntr & 0x0003;
		if (fr == 0)
		{
			ResponseStatus rs = e220ttl.sendFixedMessage(0, BEACON_01, RF_CHAN, "pon");	// Dummy
			delay(500);
			// Work only with full connection
			e220ttl.setMode(MODE_0_NORMAL);
			Serial.println(rs.getResponseDescription());
			u8g2.clearBuffer();
			u8g2.drawStr(0, 32, "BEACON_01 ON");	// 3rd line
			u8g2.sendBuffer();
		}
		else if  (fr == 1 || fr == 2 )
		{
			ResponseStatus rs = e220ttl.sendFixedMessage(0, BEACON_01, RF_CHAN, "a message");
			Serial.println(rs.getResponseDescription());
			u8g2.clearBuffer();
			u8g2.drawStr(0, 32, "message");	// 3rd line
			u8g2.sendBuffer();
			delay(500);
		}
		else if (fr == 3)
		{
			ResponseStatus rs = e220ttl.sendFixedMessage(0, BEACON_01, RF_CHAN, "poff");
			Serial.println(rs.getResponseDescription());
			u8g2.clearBuffer();
			u8g2.drawStr(0, 32, "BEACON_01 OFF");	// 3rd line
			u8g2.sendBuffer();
			e220ttl.setMode(MODE_1_WOR_TRANSMITTER);
			delay(500);
		}
	}
	if (cntr == 100)
		digitalWrite(UserLED, LOW);
}

