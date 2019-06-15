#ifndef __NRF24L01_H__
#define __NRF24L01_H__

#include "spi.h"

#define RF_CONFIG_REGISTER 0x00
#define RF_CHANNEL_REGISTER 0x05
#define RF_SETUP_REGISTER 0x06
#define RF_STATUS_REGISTER 0x07
#define RF_EN_AA_REGISTER 0x01

#define RF_READ_REGISTER_BASE 0x00
#define RF_WRITE_REGISTER_BASE 0x20

#define RF_CONFIG_PWR_UP_MASK 0x02
#define RF_CONFIG_PRIM_RX_MASK 0x01

#define RF_INTERRUPT_CLEAR_MASK 0x70

#define RF_TX_ADDRESS_REGISTER 0x10
#define RF_RX_ADRESS_P0 0x0A
#define RF_RX_PW_P0 0x11
#define RF_SETUP_AW_REGISTER 0x03

#define SETUP_AW_3_BYTES 0x01
#define SETUP_AW_4_BYTES 0x02
#define SETUP_AW_5_BYTES 0x03

#define RX_DR_BIT 0x06
#define TX_DS_BIT 0x05
#define MAX_RT_BIT 0x04

//Used to read status register as it 
//is being transmited on MISO when Master writes something on SPI
#define NOP_COMMAND 0xff
#define RF_FLUSH_RX_COMMAND 0xE2
#define RF_FLUSH_TX_COMMAND 0xE1

#define R_RX_PAYLOAD 0x61
#define W_TX_PAYLOAD 0xA0
#define R_TX_PAYLOAD_NOACK 0xB0 //No ACK

#define PAYLOAD_SIZE 5

void rf_initialize(SPI_HandleTypeDef * spi_handler,GPIO_TypeDef * gpio, uint16_t chip_select_pin);
void rf_init(void);
//flag mean 0 or 1 on LSB
void rf_set_receiver(uint8_t flag);
void rf_set_power_up(uint8_t flag);
void rf_set_tx_ds_interrupt(uint8_t flag);
void rf_initialize_address(uint8_t *address, uint8_t address_length);

void write_byte_to_register(uint8_t register_address, uint8_t data);
uint8_t read_register(uint8_t register_address);
void write_rx_address(void);
void write_tx_address(void);

void write_payload(void);
void read_payload(void);

void rf_clear_interrupt_flags(void);

void configure_as_transmitter(void);
void configure_as_receiver(void);

void clear_chip_select(void);
void set_chip_select(void);

void read_status_register(void);

void write_payload_width(void);

uint8_t * get_payload(void);
uint8_t status(void);

void flush_rx(void);
void flush_tx(void);

void write_tx_shutter_address(void);
void write_rx_shutter_address(void);

uint8_t * encode_diode(uint8_t * encodedData,struct  DiodeColor * diode_color_struct);

uint8_t isSendSuccess(void);

#endif
