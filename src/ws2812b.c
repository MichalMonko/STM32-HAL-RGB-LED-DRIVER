#include "ws2812b.h"
#include "stm32f1xx_hal.h"

SPI_HandleTypeDef * spi_handle;
struct DiodeColor diodes_colors[DIODES_NUMBER];
uint8_t encoded_color_data[DIODES_NUMBER * 9 + 2 * RESET_CODE_SIZE];
volatile uint32_t encoded_color = 0;

void init_spi(SPI_HandleTypeDef * spih)
{
	spi_handle = spih;
}

void set_diode_color(uint8_t index, uint8_t R, uint8_t G, uint8_t B)
{
	if(index < DIODES_NUMBER)
	{
	diodes_colors[index].red = R;
	diodes_colors[index].green = G;
	diodes_colors[index].blue = B;
	}
}

void send_data_to_spi(void)
{
	encode_diodes();
	HAL_SPI_Transmit(spi_handle,encoded_color_data,sizeof(encoded_color_data),HAL_MAX_DELAY);
//	while(HAL_DMA_STATE_READY != HAL_DMA_GetState(spi_handle->hdmatx));
}

void encode_diodes(void)
{
	//RESTART CODE
	int j=0;
	for(j=0; j < RESET_CODE_SIZE; j++)
	{
		encoded_color_data[j] = 0x00;
	}
	
for(int i =0;i<DIODES_NUMBER;i++)
{
	
//Green
for(uint8_t mask =0x80; mask; mask>>=1)
{
	if(diodes_colors[i].green & mask)
	{
		encoded_color |= ONE_CODE;
	}
	else
	{
		encoded_color |= ZERO_CODE;
	}
	if(mask >> 1)
	encoded_color <<=3;
}

encoded_color_data[j++] = (uint8_t)(encoded_color >> 16);
encoded_color_data[j++] = (uint8_t)(encoded_color >> 8);
encoded_color_data[j++] = (uint8_t)encoded_color;

encoded_color  = 0;
//Red
for(uint8_t mask =0x80; mask; mask>>=1)
{
	if(diodes_colors[i].red & mask)
	{
		encoded_color |= ONE_CODE;
	}
	else
	{
		encoded_color |= ZERO_CODE;
	}
	if(mask >> 1)
	encoded_color <<=3;
}

encoded_color_data[j++] = (uint8_t)(encoded_color >> 16);
encoded_color_data[j++] = (uint8_t)(encoded_color >> 8);
encoded_color_data[j++] = (uint8_t)encoded_color;

encoded_color  = 0x00;
//Blue
for(uint8_t mask =0x80; mask; mask>>=1)
{
	if(diodes_colors[i].blue & mask)
	{
		encoded_color |= ONE_CODE;
	}
	else
	{
		encoded_color |= ZERO_CODE;
	}
	if(mask >> 1)
	encoded_color <<=3;
}

encoded_color_data[j++] = (uint8_t)(encoded_color >> 16);
encoded_color_data[j++] = (uint8_t)(encoded_color >> 8);
encoded_color_data[j++] = (uint8_t)encoded_color;

}

	//RESTART CODE
	for(int i =0; i < RESET_CODE_SIZE; i++,j++)
	{
		encoded_color_data[i + j] = 0x00;
	}
}
