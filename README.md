# Projeto_final_MCC

## Módulo DHT22

  O módulo DHT22 possui 2 sensores diferentes integrados:

- Sensor de temperatura ambiente
- Sensor de umidade ambiente

  O módulo DHT22 utiliza um protocólo de comunicação similar ao protocólo One Wire, possuindo 4 pinos de conexão (somente 3 pinos são utilizados), com 1 dos pinos enviando os
dados para o microcontrolador. Esses pinos são:


**VDD** -> Pino de alimentação, pode ser alimentado de 3,3V a 5,5V.

**SDA** -> Porta Serial de comunicação bidirecional, deve ser conectada no pino a ser lido no microcontrolador.

**NC**  -> Pino não conectado.

**GND** -> Pino de terra, deve ser ligado junto a referência do microcontrolador.

## Descrição do projeto:

O projeto consiste na leitura periódica do módulo **DHT22**, para a leitura do sensor foi criado uma biblioteca de funções chamada `dht22.c`, a configuração do pino de dado *SDA* deve ser realizada nos *defines* no começo do arquivo `dht22.h` e a biblioteca possui 2 funções:

**uint8_t start_dht22()**

****void read_dht22(uint16_t* humidity, uint16_t* temperature)**

A função *start_dht22()* envia o sinal para que o dispositivo se prepare para enviar uma leitura, devendo ser feita sempre antes da função de leitura dos valores do sensor.

A função *read_dht22()* é responsável por fazer a leitura dos valores de temperatura e umidade, realizando uma checagem de erro já implementada dentro da própria função e retornando nos argumentos passados para a função os valores da temperatura.

**__Em caso de erro de leitura o valor retornado dos sensores será 999.__**

**O sensor possui leitura com uma casa decimal de precisão, porém o valor enviado não contém virgulas, devendo ser tratado pelo programador que utilizar a biblioteca**

Exemplo de utilização da função *read_dht22()* e seu resultado:

**read_dht22(umidade, temperatura);**

**retorno da função:**

**umidade = 558;**     -> valor real: **55,8%**

**temperatura = 227;** -> valor real: **22,7°C**


Após a leitura do sensor, os valores são enviados através do protocolo de comunicação *USART* para um módulo WIFI ESP-01, que recebe esses valores e os envia para um servidor na internet utilizando o protocólo MQTT (Message Queueing Telemetry Transport).

Nesse modo de comunicação é utilizado pacotes de informação RTU, contendo **8 bytes**, organizados seguindo a seguinte ordem:

**ADDR** - > É composto de 1 byte, onde é enviado o endereço do dispositivo que receberá as informações, para esse projeto é o endereço 0x15

**CMD** - > É composto de 1 byte, onde é enviado o comando que o usuário deseja enviar, para a escrita de informações deve ser utilizado 0x01 e para a leitura 0x02.

**REG** - > É composto de 2 bytes, onde é enviado o registrador que será realizado a leitura ou escrita de informações, para o projeto para atuadores as informações se encontravam do endereço 0x01 a 0x04, os sensores do endereço 0x05 a 0x08 e o estado do projeto 0x00.

**DATA** - > É composto de 2 bytes, onde é feita a leitura ou escrita de informações.

**CRC** - > É composto de 2 bytes, onde é feito o calculo do CRC (CRC é um tipo de algoritmo para detecção de erro).

Exemplo de um pacote enviado:

   **0x15 0x01 0x00 0x04 0x00 0xDF 0X2D**      ->> Envia 0x0400 para o endereço 0x05
   
O projeto realiza a escrita da leitura dos sensores periodicamente, enviando para a internet e possui *timeout* de envio de informações, para que caso algo aconteça com o circuito o programa não pare de funcionar. Além disso em caso de erro é aceso um LED de indicação de erro no envio. 


## CONTRIBUIDORES

**Eric Monteiro dos Reis**

**Jade Dutra Lopes**

