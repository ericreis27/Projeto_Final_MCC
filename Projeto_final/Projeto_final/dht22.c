/*
 * dht22.c
 *
 * Created: 07/12/2018 08:15:22
 *  Author: Eric
 */ 

#include "dht22.h"


uint8_t start_dht22(){
	
	DHT_DDR |= (1 << DHT22_DDR);		//define como saída
	_delay_us(1200);
	DHT_DDR &=~ (1 << DHT22_DDR);		//define como entrada
	_delay_us(50);
	if(!(DHT_PIN & (1 << DHT22_PIN))){
		return 1;
	}
	else {
		return 0;
	}
}

void read_dht22(uint16_t* humidity, uint16_t* temperature)
{
	uint16_t hum = 0;
	uint16_t temp = 0;
	uint8_t	 parity = 0;
	uint8_t parity_check = 0;
	uint8_t count = 0;
	
	while(!(DHT_PIN & (1 << DHT22_PIN)));
	while(DHT_PIN & (1 << DHT22_PIN));
	
	for(count = 0; count < 16; count ++){
		while(!(DHT_PIN & (1 << DHT22_PIN)));			//espera enquanto o pino está em low
		_delay_us(40);									//delay necessário para fazer a leitura no momento correto
		hum = (hum << 1);								//pega o numero atual e desloca a esquerda pra que a leitura fique na posição correta;
		if(DHT_PIN & (1 << DHT22_PIN)){					//checagem do pino
			hum|= 1;
		}
		else{
			hum |= 0;
		}
		while(DHT_PIN & (1 << DHT22_PIN));				//espera enquanto ficar em high
		
	}
	for(count = 0; count < 16; count ++){
		while(!(DHT_PIN & (1 << DHT22_PIN)));			//espera enquanto o pino está em low
		_delay_us(40);
		temp = (temp << 1);								//pega o numero atual e desloca a esquerda pra que a leitura fique na posição correta;
		if(DHT_PIN & (1 << DHT22_PIN)){					//checagem do pino
			temp |= 1;
		}
		else{
			temp |= 0;
		}
		while(DHT_PIN & (1 << DHT22_PIN));				//espera enquanto ficar em high
	}
	for(count = 0; count < 8; count++){
		while(!(DHT_PIN & (1 << DHT22_PIN)));			//espera enquanto o pino está em low
		_delay_us(40);									//delay necessário para fazer a leitura no momento correto
		parity = (parity << 1);							//pega o numero atual e desloca a esquerda pra que a leitura fique na posição correta;
		if(DHT_PIN & (1 << DHT22_PIN)){					//checagem do pino
			parity |= 1;
		}
		else{
			parity |= 0;
		}
		while(DHT_PIN & (1 << DHT22_PIN));			//espera enquanto ficar em high
	}
	
	*temperature = temp;		
	*humidity = hum;
	
	//separa as partes high e low utilizando mascara e soma em parity_check
	parity_check += (temp & (0xFF));				
	temp = (temp >> 8);							
	parity_check += temp;
	parity_check += (hum & (0xFF));
	hum = (hum >> 8);
	parity_check += hum;
	
	if(parity_check != parity){		//checa se a paridade é diferente, se for ele coloca 99.9 nos valores da saída, senão coloca o valor correto salvo previamente
		*temperature = 999;
		*humidity = 999;
	}

}