################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libraries/infineon_libraries/iLLD/TC26B/Tricore/Stm/Timer/IfxStm_Timer.c 

C_DEPS += \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/Stm/Timer/IfxStm_Timer.d 

OBJS += \
./libraries/infineon_libraries/iLLD/TC26B/Tricore/Stm/Timer/IfxStm_Timer.o 


# Each subdirectory must supply rules for building sources it contributes
libraries/infineon_libraries/iLLD/TC26B/Tricore/Stm/Timer/%.o: ../libraries/infineon_libraries/iLLD/TC26B/Tricore/Stm/Timer/%.c libraries/infineon_libraries/iLLD/TC26B/Tricore/Stm/Timer/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AURIX GCC Compiler'
	tricore-gcc -std=c99 "@D:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/External GCC - Release/AURIX_GCC_Compiler-Include_paths__-I_.opt" -O3 -Wall -c -fmessage-length=0 -fno-common -fstrict-volatile-bitfields -fdata-sections -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Stm-2f-Timer

clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Stm-2f-Timer:
	-$(RM) ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Stm/Timer/IfxStm_Timer.d ./libraries/infineon_libraries/iLLD/TC26B/Tricore/Stm/Timer/IfxStm_Timer.o

.PHONY: clean-libraries-2f-infineon_libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Stm-2f-Timer

