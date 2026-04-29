################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../code/Steering\ encoder.c \
../code/pid.c \
../code/zf_device_car_light.c \
../code/zf_device_dot_matrix_screen.c \
../code/zf_device_tld7002.c 

C_DEPS += \
./code/Steering\ encoder.d \
./code/pid.d \
./code/zf_device_car_light.d \
./code/zf_device_dot_matrix_screen.d \
./code/zf_device_tld7002.d 

OBJS += \
./code/Steering\ encoder.o \
./code/pid.o \
./code/zf_device_car_light.o \
./code/zf_device_dot_matrix_screen.o \
./code/zf_device_tld7002.o 


# Each subdirectory must supply rules for building sources it contributes
code/Steering\ encoder.o: ../code/Steering\ encoder.c code/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AURIX GCC Compiler'
	tricore-gcc -std=c99 "@D:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/External GCC - Release/AURIX_GCC_Compiler-Include_paths__-I_.opt" -O3 -Wall -c -fmessage-length=0 -fno-common -fstrict-volatile-bitfields -fdata-sections -ffunction-sections -MMD -MP -MF"code/Steering encoder.d" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

code/%.o: ../code/%.c code/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AURIX GCC Compiler'
	tricore-gcc -std=c99 "@D:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/External GCC - Release/AURIX_GCC_Compiler-Include_paths__-I_.opt" -O3 -Wall -c -fmessage-length=0 -fno-common -fstrict-volatile-bitfields -fdata-sections -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-code

clean-code:
	-$(RM) ./code/Steering\ encoder.d ./code/Steering\ encoder.o ./code/pid.d ./code/pid.o ./code/zf_device_car_light.d ./code/zf_device_car_light.o ./code/zf_device_dot_matrix_screen.d ./code/zf_device_dot_matrix_screen.o ./code/zf_device_tld7002.d ./code/zf_device_tld7002.o

.PHONY: clean-code

