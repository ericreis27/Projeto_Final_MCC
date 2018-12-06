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

int main(void)
{
	uint16_t humidity = 0;
	uint16_t temp = 0;
	uint8_t parity = 0;
	uint8_t parity_check = 0;
	uint8_t x = 0;
	uint8_t count = 0;
	FILE* lcd = 0;
	inic_LCD_4bits();
	lcd = inic_stream();
	
	DDRC |= 7;
	
	
	//tempo calculado para timeout ->  1200 inicial + 50 para leitura de low + 130 de espera até resposta + (120*40), tempo maximo de leitura do bit vezes a quantidade de bits = 6180 uS, topo 124, timer0, prescaler 1024
	while (1) 
    {
		
		DDRB |= (1 << DDB0);		//define como saída
		_delay_us(1200);
		DDRB &=~ (1 << DDB0);		//define como entrada
		_delay_us(50);
		if(!(PINB & (1 << PINB0))){
			x++;
		}

		
		if(x == 1){
			//sensor_flag = 1;
		}
		else{
			return 0;
		}
		
		
		
		
		while(!(PINB & (1 << PINB0)));
		while(PINB & (1 << PINB0));
		PORTC |= (1 << PORTC0);
		
		//leitura
		
		for(count = 0; count < 16; count ++){
			while(!(PINB & (1 << PINB0)));			//espera enquanto o pino está em low
			_delay_us(40);
			humidity = (humidity << 1);				//pega o numero atual e desloca a esquerda pra que a leitura fique na posição correta;
			if(PINB & (1 << PINB0)){
				humidity |= 1;
			}
			else{
				humidity |= 0;
			}
			while(PINB & (1 << PINB0));			//espera enquanto ficar em high			
			
		}
		PORTC |= (1 << PORTC1);
		for(count = 0; count < 16; count ++){
			while(!(PINB & (1 << PINB0)));			//espera enquanto o pino está em low
			_delay_us(40);
			temp = (temp << 1);				//pega o numero atual e desloca a esquerda pra que a leitura fique na posição correta;
			if(PINB & (1 << PINB0)){
				temp |= 1;
			}
			else{
				temp |= 0;
			}
			while(PINB & (1 << PINB0));			//espera enquanto ficar em high
		}
		PORTC |= (1 << PORTC2);
		for(count = 0; count < 8; count++){
			while(!(PINB & (1 << PINB0)));			//espera enquanto o pino está em low
			_delay_us(40);
			parity = (parity << 1);				//pega o numero atual e desloca a esquerda pra que a leitura fique na posição correta;
			if(PINB & (1 << PINB0)){
				parity |= 1;
			}
			else{
				parity |= 0;
			}
			while(PINB & (1 << PINB0));			//espera enquanto ficar em high
		}
		////////////////////////////////////////////////
		
		parity_check += temp & (0xFF);				//pega os 8 ultimos bits
		parity_check += temp & (0xFF00);			//pega os 8 primeiros bits
		parity_check += humidity & (0xFF);
		parity_check += humidity & (0xFF00);		
		
		if(parity_check == parity){
			//manda os numeros de retorno
		}
		else{
			//retorna erro na leitura
		}
		
		//mostra os valores de temperatura e humidade
		cmd_LCD(1,0);
		fprintf(lcd, "temp:%d,%d",(temp/10),(temp%10));
		cmd_LCD(0xC0, 0);
		fprintf(lcd, "hum:%d,%d",(humidity/10),(humidity%10));
		
    }
		
	
}


