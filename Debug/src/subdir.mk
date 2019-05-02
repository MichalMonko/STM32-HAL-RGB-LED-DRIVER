################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/dma.c \
../src/gpio.c \
../src/main.c \
../src/nRF24L01.c \
../src/spi.c \
../src/stm32f1xx_it.c \
../src/syscalls.c \
../src/system_stm32f1xx.c \
../src/ws2812b.c 

OBJS += \
./src/dma.o \
./src/gpio.o \
./src/main.o \
./src/nRF24L01.o \
./src/spi.o \
./src/stm32f1xx_it.o \
./src/syscalls.o \
./src/system_stm32f1xx.o \
./src/ws2812b.o 

C_DEPS += \
./src/dma.d \
./src/gpio.d \
./src/main.d \
./src/nRF24L01.d \
./src/spi.d \
./src/stm32f1xx_it.d \
./src/syscalls.d \
./src/system_stm32f1xx.d \
./src/ws2812b.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DDEBUG -DSTM32F103xB -DUSE_HAL_DRIVER -I"/home/warchlak/eclipse-workbench/HelloWorld/HAL_Driver/Inc/Legacy" -I"/home/warchlak/eclipse-workbench/HelloWorld/inc" -I"/home/warchlak/eclipse-workbench/HelloWorld/CMSIS/device" -I"/home/warchlak/eclipse-workbench/HelloWorld/CMSIS/core" -I"/home/warchlak/eclipse-workbench/HelloWorld/HAL_Driver/Inc" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


