// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _SensorNivel_H_
#define _SensorNivel_H_
#include "Arduino.h"

	#if defined(ESP32)
		#include <WiFi.h>
	#elif defined(ESP8266)
		#include <ESP8266WiFi.h>
	#endif

	#include "Security.h"

	#include <Firebase_ESP_Client.h>

	//Provide the token generation process info.
	#include <addons/TokenHelper.h>

	//Provide the RTDB payload printing info and other helper functions.
	#include <addons/RTDBHelper.h>



	// #define DATABASE_SECRET "xU6SO1uv0Q4ESkSymOr2GZoXLTsKxvfrC1iLc5kw"




//Do not add code below this line
#endif /* _SensorNivel_H_ */
