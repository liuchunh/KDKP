################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../Libraries/iLLD/TC26B/Tricore/Cif/Cam/IfxCif_Cam.c" 

COMPILED_SRCS += \
"Libraries/iLLD/TC26B/Tricore/Cif/Cam/IfxCif_Cam.src" 

C_DEPS += \
"./Libraries/iLLD/TC26B/Tricore/Cif/Cam/IfxCif_Cam.d" 

OBJS += \
"Libraries/iLLD/TC26B/Tricore/Cif/Cam/IfxCif_Cam.o" 


# Each subdirectory must supply rules for building sources it contributes
"Libraries/iLLD/TC26B/Tricore/Cif/Cam/IfxCif_Cam.src":"../Libraries/iLLD/TC26B/Tricore/Cif/Cam/IfxCif_Cam.c" "Libraries/iLLD/TC26B/Tricore/Cif/Cam/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0202.icm45686/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"Libraries/iLLD/TC26B/Tricore/Cif/Cam/IfxCif_Cam.o":"Libraries/iLLD/TC26B/Tricore/Cif/Cam/IfxCif_Cam.src" "Libraries/iLLD/TC26B/Tricore/Cif/Cam/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Cif-2f-Cam

clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Cif-2f-Cam:
	-$(RM) ./Libraries/iLLD/TC26B/Tricore/Cif/Cam/IfxCif_Cam.d ./Libraries/iLLD/TC26B/Tricore/Cif/Cam/IfxCif_Cam.o ./Libraries/iLLD/TC26B/Tricore/Cif/Cam/IfxCif_Cam.src

.PHONY: clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Cif-2f-Cam

