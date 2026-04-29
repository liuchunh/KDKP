################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuCcu.c \
../libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuEru.c \
../libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuWdt.c 

C_DEPS += \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuCcu.d \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuEru.d \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuWdt.d 

OBJS += \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuCcu.o \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuEru.o \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuWdt.o 


# Each subdirectory must supply rules for building sources it contributes
libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/%.o: ../libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/%.c libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AURIX GCC Compiler'
	tricore-gcc -std=c99 "@D:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/External GCC - Release/AURIX_GCC_Compiler-Include_paths__-I_.opt" -O3 -Wall -c -fmessage-length=0 -fno-common -fstrict-volatile-bitfields -fdata-sections -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Scu-2f-Std

clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Scu-2f-Std:
	-$(RM) ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuCcu.d ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuCcu.o ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuEru.d ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuEru.o ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuWdt.d ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/IfxScuWdt.o

.PHONY: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Scu-2f-Std

