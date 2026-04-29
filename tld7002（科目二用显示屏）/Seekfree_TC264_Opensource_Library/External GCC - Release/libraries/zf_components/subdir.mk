################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libraries/zf_components/printf_redirect.c \
../libraries/zf_components/seekfree_assistant.c \
../libraries/zf_components/seekfree_assistant_interface.c 

C_DEPS += \
./libraries/zf_components/printf_redirect.d \
./libraries/zf_components/seekfree_assistant.d \
./libraries/zf_components/seekfree_assistant_interface.d 

OBJS += \
./libraries/zf_components/printf_redirect.o \
./libraries/zf_components/seekfree_assistant.o \
./libraries/zf_components/seekfree_assistant_interface.o 


# Each subdirectory must supply rules for building sources it contributes
libraries/zf_components/%.o: ../libraries/zf_components/%.c libraries/zf_components/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AURIX GCC Compiler'
	tricore-gcc -std=c99 "@D:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/External GCC - Release/AURIX_GCC_Compiler-Include_paths__-I_.opt" -O3 -Wall -c -fmessage-length=0 -fno-common -fstrict-volatile-bitfields -fdata-sections -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-libraries-2f-zf_components

clean-libraries-2f-zf_components:
	-$(RM) ./libraries/zf_components/printf_redirect.d ./libraries/zf_components/printf_redirect.o ./libraries/zf_components/seekfree_assistant.d ./libraries/zf_components/seekfree_assistant.o ./libraries/zf_components/seekfree_assistant_interface.d ./libraries/zf_components/seekfree_assistant_interface.o

.PHONY: clean-libraries-2f-zf_components

