################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libraries/infineon_libraries/Service/CpuGeneric/If/SpiIf.c 

C_DEPS += \
./libraries/infineon_libraries/Service/CpuGeneric/If/SpiIf.d 

OBJS += \
./libraries/infineon_libraries/Service/CpuGeneric/If/SpiIf.o 


# Each subdirectory must supply rules for building sources it contributes
libraries/infineon_libraries/Service/CpuGeneric/If/%.o: ../libraries/infineon_libraries/Service/CpuGeneric/If/%.c libraries/infineon_libraries/Service/CpuGeneric/If/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AURIX GCC Compiler'
	tricore-gcc -std=c99 "@D:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/External GCC - Release/AURIX_GCC_Compiler-Include_paths__-I_.opt" -O3 -Wall -c -fmessage-length=0 -fno-common -fstrict-volatile-bitfields -fdata-sections -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-libraries-2f-infineon_libraries-2f-Service-2f-CpuGeneric-2f-If

clean-libraries-2f-infineon_libraries-2f-Service-2f-CpuGeneric-2f-If:
	-$(RM) ./libraries/infineon_libraries/Service/CpuGeneric/If/SpiIf.d ./libraries/infineon_libraries/Service/CpuGeneric/If/SpiIf.o

.PHONY: clean-libraries-2f-infineon_libraries-2f-Service-2f-CpuGeneric-2f-If

