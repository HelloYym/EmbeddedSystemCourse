//
//    FILE: dht11.cpp
// VERSION: 0.4.1
// PURPOSE: DHT11 Temperature & Humidity Sensor library for Arduino
// LICENSE: GPL v3 (http://www.gnu.org/licenses/gpl.html)
//
// DATASHEET: http://www.micro4you.com/files/sensor/DHT11.pdf
//
// HISTORY:
// George Hadjikyriacou - Original version (??)
// Mod by SimKard - Version 0.2 (24/11/2010)
// Mod by Rob Tillaart - Version 0.3 (28/03/2011)
// + added comments
// + removed all non DHT11 specific code
// + added references
// Mod by Rob Tillaart - Version 0.4 (17/03/2012)
// + added 1.0 support
// Mod by Rob Tillaart - Version 0.4.1 (19/05/2012)
// + added error codes
//

#include "dht11.h"

// Return values:
// DHTLIB_OK
// DHTLIB_ERROR_CHECKSUM
// DHTLIB_ERROR_TIMEOUT

int humidity;
int temperature;



int get_humidity() {
	return humidity;
}
int get_temperature() {
	return temperature;
}



void DHT11_Set(int state){
    BitAction s;
    if (state){
        s = Bit_SET;
    }else{
        s = Bit_RESET;
    }
    GPIO_WriteBit(DHT11_PORT, DHT11_PIN, s);
}

void DHT11_Pin_OUT(){
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);

    DHT11_Set(1);
}

void DHT11_Pin_IN(){
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);

    DHT11_Set(1);
}

void Delay_us(int times){
    unsigned int i;
    for (i=0; i<times; i++){
    }
}

int read_dht11()
{
	// BUFFER TO RECEIVE
	uint8_t bits[5];
	uint8_t cnt = 7;
	uint8_t idx = 0;
	
	unsigned int loopCnt = 10000;
	
	uint8_t sum;

	// EMPTY BUFFER
	int i;
	for (i=0; i< 5; i++) bits[i] = 0;

	// REQUEST SAMPLE	
	DHT11_Pin_OUT();
  DHT11_Set(0);
  Delay_us(25000);
  DHT11_Set(1);
  Delay_us(25);
  DHT11_Set(0);
  DHT11_Pin_IN();
	

	// ACKNOWLEDGE or TIMEOUT
	loopCnt = 10000;
	while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) == LOW)
		if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

	loopCnt = 10000;
	while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) == HIGH)
		if (loopCnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

	// READ OUTPUT - 40 BITS => 5 BYTES or TIMEOUT
	for (i=0; i<40; i++)
	{
		loopCnt = 10000;
		while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) == LOW)
			if (loopCnt-- == 0) return 555;
		
		

		loopCnt = 10000;
		while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) == HIGH) {
			if (loopCnt-- == 0) return 666;
			Delay_us(1);
		}
		
		

		if (loopCnt < 9997) bits[idx] |= (1 << cnt);
		
		if (cnt == 0)   // next byte?
		{
			cnt = 7;    // restart at MSB
			idx++;      // next byte!
		}
		else cnt--;
	}
	
	//return i;

	// WRITE TO RIGHT VARS
        // as bits[1] and bits[3] are allways zero they are omitted in formulas.
	humidity    = bits[0]; 
	temperature = bits[2]; 

	sum = bits[0] + bits[2];  

	if (bits[4] != sum) return DHTLIB_ERROR_CHECKSUM;
	return DHTLIB_OK;
}

//
// END OF FILE
//

