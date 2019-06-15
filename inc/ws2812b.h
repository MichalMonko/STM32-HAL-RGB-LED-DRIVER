
#define DIODES_NUMBER 8
#define ONE_CODE 0x06
#define ZERO_CODE 0x04
//#define ONE_CODE 0x3
//#define ZERO_CODE 0x01
#define RESET_CODE_SIZE 20

#ifndef __WS2812B_H_
#define __WS2812B_H_
#include "stdint.h"
#include "stm32f1xx_hal.h"

struct DiodeColor {
	uint8_t red, green,blue;
};

extern SPI_HandleTypeDef * spi_handle;
extern struct DiodeColor diodes_colors[DIODES_NUMBER];
extern uint8_t encoded_color_data[DIODES_NUMBER * 9 + 3];

void init_spi(SPI_HandleTypeDef * spih);
uint32_t* encodeDiodeColor(struct DiodeColor * diode_color);
void encode_diodes(void);
void set_diode_color(uint8_t diode_index, uint8_t R, uint8_t G, uint8_t B);
void send_data_to_spi(void);
#endif
