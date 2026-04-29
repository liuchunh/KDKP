################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../code/TLD7002_driver/TLD7002FuncLayer.c \
../code/TLD7002_driver/TLD7002_ControlLayer.c \
../code/TLD7002_driver/TLD7002_ServiceLayer.c 

C_DEPS += \
./code/TLD7002_driver/TLD7002FuncLayer.d \
./code/TLD7002_driver/TLD7002_ControlLayer.d \
./code/TLD7002_driver/TLD7002_ServiceLayer.d 

OBJS += \
./code/TLD7002_driver/TLD7002FuncLayer.o \
./code/TLD7002_driver/TLD7002_ControlLayer.o \
./code/TLD7002_driver/TLD7002_ServiceLayer.o 


# Each subdirectory must supply rules for building sources it contributes
code/TLD7002_driver/%.o: ../code/TLD7002_driver/%.c code/TLD7002_driver/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AURIX GCC Compiler'
	tricore-gcc -std=c99 "@D:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/External GCC - Release/AURIX_GCC_Compiler-Include_paths__-I_.opt" -O3 -Wall -c -fmessage-length=0 -fno-common -fstrict-volatile-bitfields -fdata-sections -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-code-2f-TLD7002_driver

clean-code-2f-TLD7002_driver:
	-$(RM) ./code/TLD7002_driver/TLD7002FuncLayer.d ./code/TLD7002_driver/TLD7002FuncLayer.o ./code/TLD7002_driver/TLD7002_ControlLayer.d ./code/TLD7002_driver/TLD7002_ControlLayer.o ./code/TLD7002_driver/TLD7002_ServiceLayer.d ./code/TLD7002_driver/TLD7002_ServiceLayer.o

.PHONY: clean-code-2f-TLD7002_driver

