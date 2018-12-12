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
#include "avr_usart.h"
#include <avr/interrupt.h>

uint16_t CRC16_2(uint8_t *buf, int len)
{
	uint32_t crc = 0xFFFF;
	int i;

	for (i = 0; i < len; i++)
	{
		crc ^= (uint16_t)buf[i];          // XOR byte into least sig. byte of crc

		for (int i = 8; i != 0; i--) {    // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			}
			else                            // Else LSB is not set
			crc >>= 1;                    // Just shift right
		}
	}
	// Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
	return crc;
}

typedef struct pkg{
	uint8_t addr;
	uint8_t cmd;
	uint16_t reg;
	uint16_t data;
	uint16_t crc;
} modbus_pkg_t;

// O pacote do Modbus é em Big Endian, porém os dados que estamos enviando estão em Little Endian.

uint16_t little_to_big(uint16_t input)
{
	uint16_t output;
	output = (input >> 8) + (input << 8);	// Deslocando os bits para a ordem correta do Modbus
	return output;
}

int main(void)
{
	modbus_pkg_t tx_pkg;	 // Dados que serão enviados ao Modbus
	modbus_pkg_t rx_pkg;	 // Dados que o Modbus me retornará (deverão ser os mesmos)
	FILE* lcd = 0;
	FILE* usart = 0;
	uint16_t humidity = 0;
	uint16_t temperature = 0;
	inic_LCD_4bits();
	lcd = inic_stream();
	usart = get_usart_stream();
	USART_Init(B9600);
	sei();
	uint8_t buffer[8];
	uint8_t x = 0;
	// Fazendo a transmissão dos dados
	

	// Endereço do pacote e quantos bytes são para o cálculo
	
	// Checagem dos erros do pacote da temperatura
	// ...
	
	/*tx_pkg.addr = 0x15;
	tx_pkg.cmd  = 0x01;
	tx_pkg.reg  = 0x0600;	// Big Endian
	tx_pkg.data = little_to_big(humidity);
	tx_pkg.crc  = CRC16_2(&tx_pkg, 6);	// Endereço do pacote e quantos bytes são para o cálculo
	*/
	// Checagem dos erros do pacote da umidade
	// ...
	
	//tempo calculado para timeout:
	//   1200 inicial + 50 para leitura de low + 130 de espera até resposta + (120*40)
	//   tempo máximo de leitura do bit vezes a quantidade de bits = 6180 uS
	//   topo 124, timer0, prescaler 1024
	while (1) 
    {
		if(start_dht22() == 1){
			read_dht22(&humidity, &temperature);
		}	
		tx_pkg.addr = 0x15;
		tx_pkg.cmd  = 0x01;
		tx_pkg.reg  = 0x0500;	// Big Endian
		tx_pkg.data = little_to_big(temperature);
		tx_pkg.crc  = CRC16_2((uint8_t*)&tx_pkg, 6);

		fwrite(&tx_pkg, sizeof(tx_pkg), 1, usart);		//envio do pacote
		
		for(x = 0; x < 8; x++){
			buffer[x] = USART_rx();
			//fprintf(usart, "%x\r\n", buffer[x]);
		}

		tx_pkg.addr = 0x15;
		tx_pkg.cmd  = 0x01;
		tx_pkg.reg  = 0x0600;	// Big Endian
		tx_pkg.data = little_to_big(humidity);
		tx_pkg.crc  = CRC16_2(&tx_pkg, 6);	// Endereço do pacote e quantos bytes são para o cálculo
		
		fwrite(&tx_pkg, sizeof(tx_pkg), 1, usart);		//envio do pacote

		for(x = 0; x < 8; x++){
			buffer[x] = USART_rx();
			//fprintf(usart, "%x\r\n", buffer[x]);
		}
		
		//mostra os valores de temperatura e umidade
		//cmd_LCD(1,0);
		//fprintf(lcd, "temp:%d,%d",(temperature/10),(temperature%10));
		//cmd_LCD(0xC0, 0);
		//fprintf(lcd, "hum:%d,%d",(humidity/10),(humidity%10));
		//fprintf(usart, "temp:%d,%d\r\n",(temperature/10),(temperature%10));
		//fprintf(usart, "hum:%d,%d\r\n",(humidity/10),(humidity%10));
		
//fprintf(usart, "----------------");
		_delay_ms(1000);
    }
}


