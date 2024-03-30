/*
 * LoRa E220
 * Set configuration.
 *
 * You must uncommend the correct constructor.
 *
 * by Renzo Mischianti <https://www.mischianti.org>
 *
 * https://www.mischianti.org
 *
 * E220		  ----- WeMos D1 mini	----- esp32			----- Arduino Nano 33 IoT	----- Arduino MKR	----- Raspberry Pi Pico   ----- stm32               ----- ArduinoUNO
 * M0         ----- D7 (or 3.3v)	----- 19 (or 3.3v)	----- 4 (or 3.3v)			----- 2 (or 3.3v)	----- 10 (or 3.3v)	      ----- PB0 (or 3.3v)       ----- 7 Volt div (or 3.3v)
 * M1         ----- D6 (or 3.3v)	----- 21 (or 3.3v)	----- 6 (or 3.3v)			----- 4 (or 3.3v)	----- 11 (or 3.3v)	      ----- PB10 (or 3.3v)      ----- 6 Volt div (or 3.3v)
 * TX         ----- D3 (PullUP)		----- TX2 (PullUP)	----- TX1 (PullUP)			----- 14 (PullUP)	----- 8 (PullUP)	      ----- PA2 TX2 (PullUP)    ----- 4 (PullUP)
 * RX         ----- D4 (PullUP)		----- RX2 (PullUP)	----- RX1 (PullUP)			----- 13 (PullUP)	----- 9 (PullUP)	      ----- PA3 RX2 (PullUP)    ----- 5 Volt div (PullUP)
 * AUX        ----- D5 (PullUP)		----- 18  (PullUP)	----- 2  (PullUP)			----- 0  (PullUP)	----- 2  (PullUP)	      ----- PA0  (PullUP)       ----- 3 (PullUP)
 * VCC        ----- 3.3v/5v			----- 3.3v/5v		----- 3.3v/5v				----- 3.3v/5v		----- 3.3v/5v		      ----- 3.3v/5v             ----- 3.3v/5v
 * GND        ----- GND				----- GND			----- GND					----- GND			----- GND			      ----- GND                 ----- GND
 *
 */

//**koseki(2023.1.27)
#define ADDR_L	0x00	// unique module address (controller)
#define ADDR_H	0x00	// common
#define RF_CHAN	6		// 922MHz (ch.31)

#include "Arduino.h"
#include "LoRa_E220(JP).h"
#include "device_setup.h"

void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);

void setup() {
	Serial.begin(9600);
	e220ttl.begin();

	while(!Serial){};
	delay(500);

	Serial.println("********************************");
	Serial.println("*** 02_Pico_setConfiguration ***");
	Serial.println("********************************");

	ResponseStructContainer c;
	c = e220ttl.getConfiguration();
	// It's important get configuration pointer before all other operation
	Configuration configuration = *(Configuration*) c.data;
	Serial.println(c.status.getResponseDescription());
	Serial.println(c.status.code);

	printParameters(configuration);

//	----------------------- DEFAULT TRANSPARENT -----------------------
	configuration.ADDL = ADDR_L;	// First part of address
	configuration.ADDH = ADDR_H;	// Second part

	//configuration.CHAN = 23; // Communication channel
  	configuration.CHAN = RF_CHAN;  //*Actual freq. = 922MHz (ch.31)


	configuration.SPED.uartBaudRate = UART_BPS_9600; // Serial baud rate
	configuration.SPED.airDataRate = AIR_DATA_RATE_002_148; // Air baud rate "2.148kbps SF=11, BW=500"
	//configuration.SPED.uartParity = MODE_00_8N1; // Parity bit

	configuration.OPTION.subPacketSetting = SPS_200_00; // Packet size
	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED; // Need to send special command
	//configuration.OPTION.transmissionPower = POWER_22; // Device power
	configuration.OPTION.transmissionPower = POWER_13;    //*Po = 20dBm


	//configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED; // Enable RSSI info
	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED; // **Enable RSSI info
	//configuration.TRANSMISSION_MODE.fixedTransmission = FT_TRANSPARENT_TRANSMISSION; // Enable repeater mode
	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION; // **Enable Fixed Transmission mode
	//configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED; // Check interference
	//configuration.TRANSMISSION_MODE.enableLBT = LBT_ENABLED;	// **Listen Before Trasmission
	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011; // WOR timing
//	-------------------------------------------------------------------

	// Set configuration changed and set to not hold the configuration
	ResponseStatus rs = e220ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
	Serial.println(rs.getResponseDescription());
	Serial.println(rs.code);

	c = e220ttl.getConfiguration();
	// It's important get configuration pointer before all other operation
	configuration = *(Configuration*) c.data;
	Serial.println(c.status.getResponseDescription());
	Serial.println(c.status.code);

	printParameters(configuration);
	c.close();
}

void loop() {

}

void printParameters(struct Configuration configuration) {
	Serial.println("----------------------------------------");

	Serial.print(F("HEAD : "));  Serial.print(configuration.COMMAND, HEX);Serial.print(" ");Serial.print(configuration.STARTING_ADDRESS, HEX);Serial.print(" ");Serial.println(configuration.LENGHT, HEX);
	Serial.println(F(" "));
	Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, HEX);
	Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, HEX);
	Serial.println(F(" "));
	Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
	Serial.println(F(" "));
	Serial.print(F("SpeedParityBit     : "));  Serial.print("00");Serial.print(" -> "); Serial.println("8N1 (fixed)");
	Serial.print(F("SpeedUARTDatte     : "));  Serial.print(configuration.SPED.uartBaudRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRateDescription());
	Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRateDescription());
	Serial.println(F(" "));
	Serial.print(F("OptionSubPacketSett: "));  Serial.print(configuration.OPTION.subPacketSetting, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getSubPacketSetting());
	Serial.print(F("OptionTranPower    : "));  Serial.print(configuration.OPTION.transmissionPower, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());
	Serial.print(F("OptionRSSIAmbientNo: "));  Serial.print(configuration.OPTION.RSSIAmbientNoise, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getRSSIAmbientNoiseEnable());
	Serial.println(F(" "));
	Serial.print(F("TransModeWORPeriod : "));  Serial.print(configuration.TRANSMISSION_MODE.WORPeriod, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getWORPeriodByParamsDescription());
//	Serial.print(F("TransModeEnableLBT : "));  Serial.print(configuration.TRANSMISSION_MODE.enableLBT, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getLBTEnableByteDescription());
	Serial.print(F("TransModeEnableRSSI: "));  Serial.print(configuration.TRANSMISSION_MODE.enableRSSI, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getRSSIEnableByteDescription());
	Serial.print(F("TransModeFixedTrans: "));  Serial.print(configuration.TRANSMISSION_MODE.fixedTransmission, BIN);Serial.print(" -> "); Serial.println(configuration.TRANSMISSION_MODE.getFixedTransmissionDescription());
	Serial.println("----------------------------------------");
}

void printModuleInformation(struct ModuleInformation moduleInformation) {
	Serial.println("----------------------------------------");
	Serial.print(F("HEAD: "));  Serial.print(moduleInformation.COMMAND, HEX);Serial.print(" ");Serial.print(moduleInformation.STARTING_ADDRESS, HEX);Serial.print(" ");Serial.println(moduleInformation.LENGHT, DEC);

	Serial.print(F("Model no.: "));  Serial.println(moduleInformation.model, HEX);
	Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
	Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
	Serial.println("----------------------------------------");
}