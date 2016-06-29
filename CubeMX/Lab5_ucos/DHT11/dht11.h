 

#ifndef dht11_h
#define dht11_h


#include "stm32f10x.h"
#include "stm32f10x_conf.h"


#define DHT11LIB_VERSION "0.4.1"

#define DHTLIB_OK				0
#define DHTLIB_ERROR_CHECKSUM	1
#define DHTLIB_ERROR_TIMEOUT	2

#define DHT11_PORT	GPIOC
#define DHT11_PIN 	GPIO_Pin_15

#define LOW 0
#define HIGH 1


int read_dht11();
int get_humidity();
int get_temperature();


#endif
//
// END OF FILE
//


