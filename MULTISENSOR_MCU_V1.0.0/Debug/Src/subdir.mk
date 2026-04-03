################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/main.c \
../Src/syscalls.c \
../Src/sysmem.c 

OBJS += \
./Src/main.o \
./Src/syscalls.o \
./Src/sysmem.o 

C_DEPS += \
./Src/main.d \
./Src/syscalls.d \
./Src/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F401RETx -DSTM32F4 -c -I"C:/Funix/Part 3/Assignment/IOT303x_asm2_tuongntnFX71549@funix.edu.vn/SDK_1.0.3_NUCLEO-F401RE/shared/Drivers/STM32F401RE_StdPeriph_Driver/inc" -I../Inc -I"C:/Funix/Part 3/Assignment/IOT303x_asm2_tuongntnFX71549@funix.edu.vn/SDK_1.0.3_NUCLEO-F401RE/shared/Drivers/CMSIS/Include" -I"C:/Funix/Part 3/Assignment/IOT303x_asm2_tuongntnFX71549@funix.edu.vn/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/button" -I"C:/Funix/Part 3/Assignment/IOT303x_asm2_tuongntnFX71549@funix.edu.vn/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/buzzer" -I"C:/Funix/Part 3/Assignment/IOT303x_asm2_tuongntnFX71549@funix.edu.vn/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/flash" -I"C:/Funix/Part 3/Assignment/IOT303x_asm2_tuongntnFX71549@funix.edu.vn/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/led" -I"C:/Funix/Part 3/Assignment/IOT303x_asm2_tuongntnFX71549@funix.edu.vn/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/rtos" -I"C:/Funix/Part 3/Assignment/IOT303x_asm2_tuongntnFX71549@funix.edu.vn/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/sensor" -I"C:/Funix/Part 3/Assignment/IOT303x_asm2_tuongntnFX71549@funix.edu.vn/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/serial" -I"C:/Funix/Part 3/Assignment/IOT303x_asm2_tuongntnFX71549@funix.edu.vn/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/ucglib" -I"C:/Funix/Part 3/Assignment/IOT303x_asm2_tuongntnFX71549@funix.edu.vn/SDK_1.0.3_NUCLEO-F401RE/shared/Utilities" -I"C:/Funix/Part 3/Assignment/IOT303x_asm2_tuongntnFX71549@funix.edu.vn/Kalman_filter" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su

.PHONY: clean-Src

