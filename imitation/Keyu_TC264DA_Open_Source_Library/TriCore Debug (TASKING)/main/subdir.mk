################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../main/Cpu0_Main.c" \
"../main/Cpu1_Main.c" \
"../main/Isr.c" 

COMPILED_SRCS += \
"main/Cpu0_Main.src" \
"main/Cpu1_Main.src" \
"main/Isr.src" 

C_DEPS += \
"./main/Cpu0_Main.d" \
"./main/Cpu1_Main.d" \
"./main/Isr.d" 

OBJS += \
"main/Cpu0_Main.o" \
"main/Cpu1_Main.o" \
"main/Isr.o" 


# Each subdirectory must supply rules for building sources it contributes
"main/Cpu0_Main.src":"../main/Cpu0_Main.c" "main/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"main/Cpu0_Main.o":"main/Cpu0_Main.src" "main/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"main/Cpu1_Main.src":"../main/Cpu1_Main.c" "main/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"main/Cpu1_Main.o":"main/Cpu1_Main.src" "main/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"main/Isr.src":"../main/Isr.c" "main/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"main/Isr.o":"main/Isr.src" "main/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-main

clean-main:
	-$(RM) ./main/Cpu0_Main.d ./main/Cpu0_Main.o ./main/Cpu0_Main.src ./main/Cpu1_Main.d ./main/Cpu1_Main.o ./main/Cpu1_Main.src ./main/Isr.d ./main/Isr.o ./main/Isr.src

.PHONY: clean-main

