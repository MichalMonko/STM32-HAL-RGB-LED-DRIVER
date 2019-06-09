#include "nRF24L01.h"

uint8_t last_status_data;
SPI_HandleTypeDef * spi;
uint16_t gpio_chip_select;
GPIO_TypeDef *gpio_handler;

uint8_t rf_configuration = 0x08;
//Only first byte matters
uint8_t status_register[2];
uint8_t payload[PAYLOAD_SIZE + 1] = {0x00,0xff,0xff,0xff};
uint8_t dump_data[PAYLOAD_SIZE + 1] = {0x00,0x00,0x00,0x00};
uint8_t address[5] = {0xff,0xff,0xff,0x00,0x00};
uint8_t address_length = 5;

uint8_t address_tx_write_command[6] = {0x30,0x00,0x00,0x00,0x00,0x00};
uint8_t address_rx_write_command[6] = {0x2A,0x00,0x00,0x00,0x00,0x00};

void rf_initialize(SPI_HandleTypeDef * spi_handler,GPIO_TypeDef * gpio, uint16_t chip_select_pin)
{
	spi = spi_handler;
	gpio_chip_select = chip_select_pin;
	gpio_handler = gpio;
}

void rf_set_receiver(uint8_t flag)
{
	rf_configuration &= ~(0x01);
	rf_configuration |= (flag<<0);
}
void rf_set_power_up(uint8_t flag)
{
	rf_configuration &= ~(0x02);
	rf_configuration |= (flag<<1);
}
void rf_set_tx_ds_interrupt(uint8_t flag)
{
	rf_configuration &= ~(0x20);
	rf_configuration |= (flag<<5);
}

void configure_as_transmitter(void)
{
	//Clock jest w stanie wysokim na starcie, wysyłam NOPa żeby go ściągnąć w dół
	read_status_register();
	HAL_Delay(1);

	rf_set_receiver(0x00);
	rf_set_power_up(0x01);
	
	write_byte_to_register(RF_CONFIG_REGISTER, rf_configuration);
	
	HAL_Delay(1);
	if(address_length == 5)
	write_byte_to_register(RF_SETUP_AW_REGISTER, SETUP_AW_5_BYTES);
	else if(address_length ==4)
	write_byte_to_register(RF_SETUP_AW_REGISTER, SETUP_AW_4_BYTES);
	else
	write_byte_to_register(RF_SETUP_AW_REGISTER, SETUP_AW_3_BYTES);
	HAL_Delay(1);

	HAL_Delay(1);
	write_rx_address();
	HAL_Delay(1);
	write_tx_address();
	HAL_Delay(1);
	write_payload_width();

//	HAL_Delay(1);
//	write_byte_to_register(RF_EN_AA_REGISTER,0x3E);

}

void configure_as_receiver(void) 
{
	rf_set_receiver(0x01);
	rf_set_power_up(0x01);
	
	read_status_register();

	HAL_Delay(1);
	write_byte_to_register(RF_CONFIG_REGISTER, rf_configuration);
	HAL_Delay(1);
	
	if(address_length == 5)
	write_byte_to_register(RF_SETUP_AW_REGISTER, SETUP_AW_5_BYTES);
	else if(address_length ==4)
	write_byte_to_register(RF_SETUP_AW_REGISTER, SETUP_AW_4_BYTES);
	else
	write_byte_to_register(RF_SETUP_AW_REGISTER, SETUP_AW_3_BYTES);
	HAL_Delay(1);

	HAL_Delay(1);
	write_rx_address();
	HAL_Delay(1);
	write_payload_width();
	HAL_Delay(1);
}

void write_byte_to_register(uint8_t register_address, uint8_t data)
{
	uint8_t command_data[2];
	command_data[0] = (RF_WRITE_REGISTER_BASE | register_address);
	command_data[1]	= data;
	
	clear_chip_select();
	HAL_SPI_TransmitReceive(spi, command_data,status_register,2,HAL_MAX_DELAY);
	set_chip_select();
}

uint8_t read_register(uint8_t register_address)
{
	uint8_t command_data[2];
	command_data[0] = (RF_READ_REGISTER_BASE | register_address);
	command_data[1]	= 0x00;

	clear_chip_select();
	HAL_SPI_TransmitReceive(spi, command_data,status_register,2,HAL_MAX_DELAY);
	set_chip_select();

	return status_register[1];
}

void rf_initialize_address(uint8_t *address, uint8_t address_length)
{
	for(uint8_t i = 1; i <= address_length; i++)
	{
		address_rx_write_command[i] = address[i-1];
		address_tx_write_command[i] = address[i-1];
	}
}

void write_rx_address()
{
	clear_chip_select();
	HAL_SPI_Transmit(spi,address_rx_write_command, address_length + 1,HAL_MAX_DELAY);
	set_chip_select();
}

void write_tx_address()
{
	clear_chip_select();
	HAL_SPI_Transmit(spi,address_tx_write_command, address_length + 1,HAL_MAX_DELAY);
	set_chip_select();
}

void write_payload_width()
{
	write_byte_to_register(RF_RX_PW_P0,PAYLOAD_SIZE);
}

void read_status_register(void) {
	//read_status_register
	uint8_t data = NOP_COMMAND;

	clear_chip_select();
	HAL_SPI_TransmitReceive(spi,&data,status_register,1,HAL_MAX_DELAY);
	set_chip_select();
}

uint8_t status() {
	return last_status_data;
}

void rf_clear_interrupt_flags(void) 
{
	//read_status_register
//	read_status_register();
//	last_status_data = status_register[0];
	//HAL_Delay(1);
	write_byte_to_register(RF_STATUS_REGISTER,0xff);
}

void clear_chip_select(void)
{
	HAL_GPIO_WritePin(gpio_handler,gpio_chip_select,GPIO_PIN_RESET);
}
void set_chip_select(void)
{
	HAL_GPIO_WritePin(gpio_handler,gpio_chip_select,GPIO_PIN_SET);
}

void write_payload(void) 
{
	HAL_Delay(1);
	payload[0] = W_TX_PAYLOAD;
	clear_chip_select();
	HAL_SPI_TransmitReceive(spi,payload,dump_data,sizeof(payload),HAL_MAX_DELAY);
	set_chip_select();
}

void read_payload(void)
{
	HAL_Delay(1);
	dump_data[0] = R_RX_PAYLOAD;
	clear_chip_select();
	HAL_SPI_TransmitReceive(spi,dump_data,payload,sizeof(payload),HAL_MAX_DELAY);
	set_chip_select();
}

uint8_t * get_payload()
{
	return payload;
}

void flush_rx()
{
	uint8_t data = RF_FLUSH_RX_COMMAND;
	set_chip_select();
	HAL_SPI_TransmitReceive(spi,&data,status_register,1,HAL_MAX_DELAY);
	clear_chip_select();
}
