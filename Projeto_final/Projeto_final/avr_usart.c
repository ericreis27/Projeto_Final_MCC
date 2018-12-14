/*
 * avr_usart.c
 *
 *  Created on: Mar 27, 2018
 *      Author: Renan Augusto Starke
 *      Instituto Federal de Santa Catarina
 */

#include <stdio.h>
#include <avr/interrupt.h>
#include "avr_usart.h"
#include "bits.h"

static int usart_putchar(char c, FILE *fp);
static uint8_t buffer[16];
static uint8_t head = 0;
static uint8_t tail = 0;

/* Stream init for printf  */
FILE usart_str = FDEV_SETUP_STREAM(usart_putchar, NULL, _FDEV_SETUP_WRITE);

/* Return stream pointer  */
FILE * get_usart_stream(){
	return &usart_str;
}

void USART_Init(uint16_t bauds){

	USART_0->UBRR_H = (uint8_t) (bauds >> 8);
	USART_0->UBRR_L = bauds;

	/* Disable double speed  */
	USART_0->UCSR_A = 0;
	/* Enable TX and RX */
	USART_0->UCSR_B = SET(RXEN0) | SET(TXEN0) | SET(RXCIE0);
	/* Asynchronous mode:
	 * - 8 data bits
	 * - 1 stop bit
	 * - no parity 	 */
	USART_0->UCSR_C = SET(UCSZ01) | SET(UCSZ00);
}


/* Send one byte: busy waiting */
void USART_tx(uint8_t data) {

	/* Wait until hardware is ready */
	while (!(USART_0->UCSR_A & (1 << UDRE0)));

	USART_0->UDR_ = data;
}

/* Receive one byte: busy waiting */
uint8_t USART_rx() {

	/* Wait until something arrive  */
	while (!(USART_0->UCSR_A & (1 << RXC0)));

	return USART_0->UDR_;
}


static int usart_putchar(char c, FILE *fp){
	USART_tx(c);

	return 0;
}

// Buffer circular na interrupção do Rx
// UDR envia 1 byte, salva no buffer circular.
// Novos dados vão sobre-escrevendo dados antigos quando atingem o tamanho limite do buffer.
// Variáveis "cabeça" e "cauda" para identificar o começo e o fim do dado sendo lido.

ISR(USART_RX_vect){
	
buffer[head] = UDR0;            // Salva o valor recebido em rx no buffer
	head++;
	if(head == 16){
		head = 0;
	}
	if(head == tail){
		tail++;
		if(tail == 16){
			tail = 0;
		}
	}	
}

uint8_t usart_buffer_has_data()
{
	if(tail != head){
		return 1;
	}
	
	return 0;
}

uint8_t usart_buffer_get_data()
{
	uint8_t aux;
	aux = buffer[tail];
	tail++;
	
	if(tail == 16)
		tail = 0;
	
	return aux;	
}