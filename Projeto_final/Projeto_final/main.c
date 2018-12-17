/*
 * Projeto_final.c
 *
 * Created: 01/12/2018 08:27:49
 * Author : Eric e Jade
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "avr_gpio.h"
#include "bits.h"
#include "dht22.h"
#include "avr_usart.h"
#include <avr/interrupt.h>
#include "avr_timer.h"

#define SEND_TEMP	1
#define RECV_TEMP	2
#define SEND_HUM	3
#define RECV_HUM	4

#define CRC_ERROR	1
#define REG_ERROR	2
#define GARBAGE		3

volatile uint8_t timeout;

void timer1_init()
{
	TIMER_1->OCRA = 15624;	// Até onde o timer vai contar
	TIMER_1->TCCRA = 0;		// Não tem PWM
	TIMER_1->TCCRB = SET(WGM12);	// Retirado os regs do Prescaler para o timer não contar
	TIMER_IRQS->TC1.MASK = SET(OCIE1A);
}

// Queremos usar o timeout apenas na recepção dos dados
void timeout_start()
{
	timeout = 0;
	TIMER_1->TCNT = 0;	// Zerar o contador para ele começar a contar do zero sempre que o timeout começar
	TIMER_1->TCCRB	|= SET(CS12) | SET(CS10); 	// Prescaler de 1024
}

void timeout_stop()
{
	TIMER_1->TCCRB &= ~(SET(CS12) | SET(CS10));
}

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

uint8_t detect_pkg_error(modbus_pkg_t* rx_pkg)
{
	if(rx_pkg->cmd == 0xFF && rx_pkg->reg == 0x4F60)
		return CRC_ERROR;
	
	if(rx_pkg->cmd == 0xFE && rx_pkg->reg == 0x8EA0)
		return REG_ERROR;
		
	return GARBAGE;
}

int main(void)
{	
	modbus_pkg_t tx_pkg;	 // Dados que serão enviados ao Modbus
	modbus_pkg_t rx_pkg;	// Dados recebidos (usados para checagem de erro)
	FILE* usart = 0;
	uint8_t check;
	uint8_t flag_error = 0;
	uint8_t index = 0;
	uint8_t * pkg_byte = (uint8_t *)&tx_pkg;	// Pkg_byte contém o endereço do pacote que foi enviado (vetor de bytes)
	uint8_t * pkg_byte_rx = (uint8_t *)&rx_pkg;
	uint8_t state = SEND_TEMP;
	uint16_t humidity = 0;
	uint16_t temperature = 0;
	DDRB |= SET(PB5);
	DDRB |= SET(PB1);
	DDRB |= SET(PB2);
	DDRB |= SET(PB3);
	usart = get_usart_stream();
	USART_Init(B9600);
	timer1_init();
	sei();

	while (1) 
    {
		switch(state){
			
		case SEND_TEMP:
			if(start_dht22() == 1){
				read_dht22(&humidity, &temperature);
			}
			_delay_ms(1000);
			// Preparando o pacote
			tx_pkg.addr = 0x15;
			tx_pkg.cmd  = 0x01;
			tx_pkg.reg  = 0x0500;	// Big Endian
			tx_pkg.data = little_to_big(temperature);
			tx_pkg.crc  = little_to_big(CRC16_2((uint8_t *)&tx_pkg, 6));	// Endereço do pacote e quantos bytes são para o cálculo
			
			// Enviando o pacote
			fwrite(&tx_pkg, sizeof(tx_pkg), 1, usart);
			index = 0;
			flag_error = 0;
			state = RECV_TEMP;
			timeout_start();
			break;
			
		case RECV_TEMP:
			
			if(usart_buffer_has_data()){
				pkg_byte_rx[index] = usart_buffer_get_data();
				if(pkg_byte[index] == pkg_byte_rx[index] && flag_error == 0){
					index++;
					if(index == 8){
						state = SEND_HUM;
					}	
				}
				else{
					flag_error = 1;
					index++;
					if(index == 4){
						check = detect_pkg_error(&rx_pkg);
					
						if(check == CRC_ERROR)
							PORTB |= SET(PB1);
					
						if(check == REG_ERROR)
							PORTB |= SET(PB2);
						
						if(check == GARBAGE)
							PORTB |= SET(PB3);
						
						state = SEND_HUM;
					}
						
				}
			}
			
			// Se der timeout, é porquê demorou para receber o dado
			if(timeout){
				timeout_stop();
				PORTB |= (1 << PB5);
				state = SEND_HUM;
			}
			
			break;

		case SEND_HUM:
			_delay_ms(1000);
			// Preparando o pacote
			tx_pkg.addr = 0x15;
			tx_pkg.cmd  = 0x01;
			tx_pkg.reg  = 0x0600;	// Big Endian
			tx_pkg.data = little_to_big(humidity);
			tx_pkg.crc  = little_to_big(CRC16_2((uint8_t *)&tx_pkg, 6));	// Endereço do pacote e quantos bytes são para o cálculo
		
			// Enviando o pacote
			fwrite(&tx_pkg, sizeof(tx_pkg), 1, usart);
			index = 0;
			flag_error = 0;
			state = RECV_HUM;
			timeout_start();
			break;
		
		case RECV_HUM:
			
			if(usart_buffer_has_data()){
				if(pkg_byte[index] == usart_buffer_get_data() && flag_error == 0){
					index++;
					if(index == 8){
						state = SEND_TEMP;
						timeout_stop();
					}
				}
				else{
					flag_error = 1;
					index++;
					if(index == 4){
						check = detect_pkg_error(&rx_pkg);
						
						if(check == CRC_ERROR)
						PORTB |= SET(PB1);
						
						if(check == REG_ERROR)
						PORTB |= SET(PB2);
						
						if(check == GARBAGE)
						PORTB |= SET(PB3);
						
						state = SEND_HUM;
					}
				}
			}	
			// Se der timeout, é porquê demorou para receber o dado
			if(timeout){
				timeout_stop();
				PORTB |= (1 << PB5);
				state = SEND_TEMP;
			}
			break;	
		}
    }
}
		
ISR(TIMER1_COMPA_vect)
{
	timeout = 1;
}


