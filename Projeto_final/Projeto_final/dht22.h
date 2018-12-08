/*
 * dht22.h
 *
 * Created: 07/12/2018 08:15:37
 *  Author: Eric
 */ 

#include <avr/io.h>
#include <util/delay.h>

#ifndef DHT22_H_
#define DHT22_H_
#define		DHT_DDR		DDRB
#define		DHT22_DDR	DDB0
#define		DHT22_PORT	PORTB0
#define		DHT22_PIN	PINB0
#define		DHT_PIN		PINB

uint8_t start_dht22();
void read_dht22(uint16_t *humidity, uint16_t *temperature);





#endif /* DHT22_H_ */