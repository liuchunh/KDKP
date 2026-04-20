################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../Libraries/iLLD/TC26B/Tricore/Dsadc/Rdc/IfxDsadc_Rdc.c" 

COMPILED_SRCS += \
"Libraries/iLLD/TC26B/Tricore/Dsadc/Rdc/IfxDsadc_Rdc.src" 

C_DEPS += \
"./Libraries/iLLD/TC26B/Tricore/Dsadc/Rdc/IfxDsadc_Rdc.d" 

OBJS += \
"Libraries/iLLD/TC26B/Tricore/Dsadc/Rdc/IfxDsadc_Rdc.o" 


# Each subdirectory must supply rules for building sources it contributes
"Libraries/iLLD/TC26B/Tricore/Dsadc/Rdc/IfxDsadc_Rdc.src":"../Libraries/iLLD/TC26B/Tricore/Dsadc/Rdc/IfxDsadc_Rdc.c" "Libraries/iLLD/TC26B/Tricore/Dsadc/Rdc/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"Libraries/iLLD/TC26B/Tricore/Dsadc/Rdc/IfxDsadc_Rdc.o":"Libraries/iLLD/TC26B/Tricore/Dsadc/Rdc/IfxDsadc_Rdc.src" "Libraries/iLLD/TC26B/Tricore/Dsadc/Rdc/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Dsadc-2f-Rdc

clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Dsadc-2f-Rdc:
	-$(RM) ./Libraries/iLLD/TC26B/Tricore/Dsadc/Rdc/IfxDsadc_Rdc.d ./Libraries/iLLD/TC26B/Tricore/Dsadc/Rdc/IfxDsadc_Rdc.o ./Libraries/iLLD/TC26B/Tricore/Dsadc/Rdc/IfxDsadc_Rdc.src

.PHONY: clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Dsadc-2f-Rdc

