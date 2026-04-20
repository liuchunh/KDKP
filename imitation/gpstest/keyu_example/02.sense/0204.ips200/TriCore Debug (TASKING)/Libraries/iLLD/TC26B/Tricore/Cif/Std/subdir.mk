################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../Libraries/iLLD/TC26B/Tricore/Cif/Std/IfxCif.c" 

COMPILED_SRCS += \
"Libraries/iLLD/TC26B/Tricore/Cif/Std/IfxCif.src" 

C_DEPS += \
"./Libraries/iLLD/TC26B/Tricore/Cif/Std/IfxCif.d" 

OBJS += \
"Libraries/iLLD/TC26B/Tricore/Cif/Std/IfxCif.o" 


# Each subdirectory must supply rules for building sources it contributes
"Libraries/iLLD/TC26B/Tricore/Cif/Std/IfxCif.src":"../Libraries/iLLD/TC26B/Tricore/Cif/Std/IfxCif.c" "Libraries/iLLD/TC26B/Tricore/Cif/Std/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"Libraries/iLLD/TC26B/Tricore/Cif/Std/IfxCif.o":"Libraries/iLLD/TC26B/Tricore/Cif/Std/IfxCif.src" "Libraries/iLLD/TC26B/Tricore/Cif/Std/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Cif-2f-Std

clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Cif-2f-Std:
	-$(RM) ./Libraries/iLLD/TC26B/Tricore/Cif/Std/IfxCif.d ./Libraries/iLLD/TC26B/Tricore/Cif/Std/IfxCif.o ./Libraries/iLLD/TC26B/Tricore/Cif/Std/IfxCif.src

.PHONY: clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Cif-2f-Std

