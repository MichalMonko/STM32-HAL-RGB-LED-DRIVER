#include "ws2812b.h"
#include "stm32f1xx_hal.h"

SPI_HandleTypeDef * spi_handle;
struct DiodeColor diodes_colors[DIODES_NUMBER];
uint8_t encoded_color_data[DIODES_NUMBER * 9 + 3] = {0};
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

void encode_diode(uint8_t * encodedData,struct  DiodeColor * diode_color_struct)
{
	int i = 8;
	uint32_t color_code = 0;
	for(uint8_t mask =0x80; mask; mask>>=1)
	{
		if(diode_color_struct->green & mask)
		{
			color_code |= (ONE_CODE << (i * 3 -1));
		}
		else
		{
			color_code |= (ZERO_CODE << (i * 3 -1));
		}
		i--;
	}
	color_code >>= 2;

	*encodedData = (color_code >> 16)  & 0xff; encodedData++;
	*encodedData = (color_code >> 8) & 0xff; encodedData++;
	*encodedData= color_code & 0xff; encodedData++;

	i = 8;
	color_code = 0;
	for(uint8_t mask =0x80; mask; mask>>=1)
	{
		if(diode_color_struct->red & mask)
		{
			color_code |= (ONE_CODE << (i * 3 -1));
		}
		else
		{
			color_code |= (ZERO_CODE << (i * 3 -1));
		}
		i--;
	}
	color_code >>= 2;

	*encodedData = (color_code >> 16)  & 0xff; encodedData++;
	*encodedData = (color_code >> 8) & 0xff; encodedData++;
	*encodedData= color_code & 0xff; encodedData++;

	i = 8;
	color_code = 0;
	for(uint8_t mask =0x80; mask; mask>>=1)
	{
		if(diode_color_struct->blue & mask)
		{
			color_code |= (ONE_CODE << (i * 3 -1));
		}
		else
		{
			color_code |= (ZERO_CODE << (i * 3 -1));
		}
		i--;
	}
	color_code >>= 2;

	*encodedData = (color_code >> 16)  & 0xff; encodedData++;
	*encodedData = (color_code >> 8) & 0xff; encodedData++;
	*encodedData= color_code & 0xff; encodedData++;

}

void send_data_to_spi(void)
{
		encode_diodes();
		HAL_SPI_Transmit(spi_handle,encoded_color_data,sizeof(encoded_color_data),HAL_MAX_DELAY);
}


void encode_diodes(void)
{
	uint8_t * data = encoded_color_data;
	*data = 0; data++;
	struct DiodeColor * diodeColor = diodes_colors;
	for(int i =0; i < DIODES_NUMBER; i++)
	{
			encode_diode(data,diodeColor);
			diodeColor++;
			data += 9;
	}
}
