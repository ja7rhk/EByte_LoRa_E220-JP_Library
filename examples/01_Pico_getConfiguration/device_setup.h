// device_setup.h
#ifndef __e220_device_setup_h__
	#define __e220_device_setup_h__
/*
	#ifdef ARDUINO_RASPBERRY_PI_PICO
		// ---------- Raspberry PI Pico pins ------------------------
		// LoRa_E220 e220ttl(&Serial2, 2, 10, 11); //  RX AUX M0 M1
		// ----------------------------------------------------------
		#define M0 10
		#define M1 11
		#define AUX 12
		#define TX1 8		// ext Pull-Up 9.1kohm
		#define RX1 9		// ext Pull-Up 9.1kohm
		//Serial2
		//	UART1_TX/GP8
		//	UART1_RX/GP9
		LoRa_E220 e220ttl(&Serial2, AUX, M0, M1); // HardSerial# AUX M0 M1
		#define UserLED 22
	#endif
*/
	//#ifdef ARDUINO_AVR_NANO 
		// ---------- Arduino Nano pins --------------
		//**koseki(2023.1.29)
		#define M0 7
		#define M1 6
		#define AUX 3
		#define RX 4
		#define TX 5
		LoRa_E220 e220ttl(RX, TX, AUX, M0, M1); // Arduino RX <-- e22 TX, Arduino TX --> e22 RX AUX M0 M1
		#define UserLED 13
		// -------------------------------------
	//#endif

	typedef struct Message {
		char type[4];		// command type : POW / HIT / BAT
		char beacon[10];	// BEACON_NN
		byte value[2];		// value (0xFFFF)
		byte frame[2];		// frame ID (0xFF)
	} messages;

	typedef enum {  // command type
		power_sw = 0,
		prove_hit,
		battery
	} command_type_t;

	typedef enum {  // beacon
		beacon_00 = 0,
		beacon_01,
		beacon_02,
		beacon_03,
		beacon_04,
		beacon_05,
		beacon_06,
		beacon_07,
		beacon_08
	} beacon_t;




#endif // __e220_device_setup_h__
