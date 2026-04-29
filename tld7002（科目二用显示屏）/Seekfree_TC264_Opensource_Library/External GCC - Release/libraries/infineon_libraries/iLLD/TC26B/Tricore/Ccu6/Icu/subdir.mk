################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libraries/infineon_libraries/iLLD/TC26B/Tricore/Ccu6/Icu/IfxCcu6_Icu.c 

C_DEPS += \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/Ccu6/Icu/IfxCcu6_Icu.d 

OBJS += \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/Ccu6/Icu/IfxCcu6_Icu.o 


# Each subdirectory must supply rules for building sources it contributes
libraries/infineon_libraries/iLLD/TC26B/Tricore/Ccu6/Icu/%.o: ../libraries/infineon_libraries/iLLD/TC26B/Tricore/Ccu6/Icu/%.c libraries/infineon_libraries/iLLD/TC26B/Tricore/Ccu6/Icu/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AURIX GCC Compiler'
	tricore-gcc -std=c99 "@D:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/External GCC - Release/AURIX_GCC_Compiler-Include_paths__-I_.opt" -O3 -Wall -c -fmessage-length=0 -fno-common -fstrict-volatile-bitfields -fdata-sections -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Ccu6-2f-Icu

clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Ccu6-2f-Icu:
	-$(RM) ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Ccu6/Icu/IfxCcu6_Icu.d ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Ccu6/Icu/IfxCcu6_Icu.o

.PHONY: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Ccu6-2f-Icu

