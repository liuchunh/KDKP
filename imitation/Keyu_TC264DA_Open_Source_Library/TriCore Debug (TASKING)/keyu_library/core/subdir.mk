################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../keyu_library/core/ky_sys_clock.c" 

COMPILED_SRCS += \
"keyu_library/core/ky_sys_clock.src" 

C_DEPS += \
"./keyu_library/core/ky_sys_clock.d" 

OBJS += \
"keyu_library/core/ky_sys_clock.o" 


# Each subdirectory must supply rules for building sources it contributes
"keyu_library/core/ky_sys_clock.src":"../keyu_library/core/ky_sys_clock.c" "keyu_library/core/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/core/ky_sys_clock.o":"keyu_library/core/ky_sys_clock.src" "keyu_library/core/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-keyu_library-2f-core

clean-keyu_library-2f-core:
	-$(RM) ./keyu_library/core/ky_sys_clock.d ./keyu_library/core/ky_sys_clock.o ./keyu_library/core/ky_sys_clock.src

.PHONY: clean-keyu_library-2f-core

