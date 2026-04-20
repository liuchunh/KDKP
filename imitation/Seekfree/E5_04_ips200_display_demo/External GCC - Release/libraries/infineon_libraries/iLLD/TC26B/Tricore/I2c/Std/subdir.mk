################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libraries/infineon_libraries/iLLD/TC26B/Tricore/I2c/Std/IfxI2c.c 

C_DEPS += \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/I2c/Std/IfxI2c.d 

OBJS += \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/I2c/Std/IfxI2c.o 


# Each subdirectory must supply rules for building sources it contributes
libraries/infineon_libraries/iLLD/TC26B/Tricore/I2c/Std/%.o: ../libraries/infineon_libraries/iLLD/TC26B/Tricore/I2c/Std/%.c libraries/infineon_libraries/iLLD/TC26B/Tricore/I2c/Std/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AURIX GCC Compiler'
	tricore-gcc -std=c99 "@D:/kadingkuaipao/TC264_Library-master/TC264_Library-master/Example/Example_general/Motherboard_Demo/Motherboard_Demo/E5_display/E5_04_ips200_display_demo/External GCC - Release/AURIX_GCC_Compiler-Include_paths__-I_.opt" -O3 -Wall -c -fmessage-length=0 -fno-common -fstrict-volatile-bitfields -fdata-sections -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-I2c-2f-Std

clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-I2c-2f-Std:
	-$(RM) ./libraries/infineon_libraries/iLLD/TC26B/Tricore/I2c/Std/IfxI2c.d ./libraries/infineon_libraries/iLLD/TC26B/Tricore/I2c/Std/IfxI2c.o

.PHONY: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-I2c-2f-Std

