/*
 * Projeto_final.c
 *
 * Created: 01/12/2018 08:27:49
 * Author : Eric
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
#include "avr_gpio.h"
#include "bits.h"
#include "dht22.h"


int main(void)
{
	FILE* lcd = 0;
	uint16_t humidity = 0;
	uint16_t temperature = 0;
	inic_LCD_4bits();
	lcd = inic_stream();
	
	//tempo calculado para timeout ->  1200 inicial + 50 para leitura de low + 130 de espera até resposta + (120*40), tempo maximo de leitura do bit vezes a quantidade de bits = 6180 uS, topo 124, timer0, prescaler 1024
	while (1) 
    {
		
		if(start_dht22() == 1){
			read_dht22(&humidity, &temperature);
		}
		
		//mostra os valores de temperatura e humidade
		cmd_LCD(1,0);
		fprintf(lcd, "temp:%d,%d",(temperature/10),(temperature%10));
		cmd_LCD(0xC0, 0);
		fprintf(lcd, "hum:%d,%d",(humidity/10),(humidity%10));
		_delay_ms(100);
		
    }
		
	
}


