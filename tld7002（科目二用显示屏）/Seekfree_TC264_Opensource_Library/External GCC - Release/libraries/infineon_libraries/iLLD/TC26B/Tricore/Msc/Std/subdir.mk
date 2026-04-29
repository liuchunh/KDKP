################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libraries/infineon_libraries/iLLD/TC26B/Tricore/Msc/Std/IfxMsc.c 

C_DEPS += \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/Msc/Std/IfxMsc.d 

OBJS += \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/Msc/Std/IfxMsc.o 


# Each subdirectory must supply rules for building sources it contributes
libraries/infineon_libraries/iLLD/TC26B/Tricore/Msc/Std/%.o: ../libraries/infineon_libraries/iLLD/TC26B/Tricore/Msc/Std/%.c libraries/infineon_libraries/iLLD/TC26B/Tricore/Msc/Std/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AURIX GCC Compiler'
	tricore-gcc -std=c99 "@D:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/External GCC - Release/AURIX_GCC_Compiler-Include_paths__-I_.opt" -O3 -Wall -c -fmessage-length=0 -fno-common -fstrict-volatile-bitfields -fdata-sections -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Msc-2f-Std

clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Msc-2f-Std:
	-$(RM) ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Msc/Std/IfxMsc.d ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Msc/Std/IfxMsc.o

.PHONY: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Msc-2f-Std

