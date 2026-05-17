################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../code/ins/ins_solver.c" \
"../code/ins/math_utils.c" 

COMPILED_SRCS += \
"code/ins/ins_solver.src" \
"code/ins/math_utils.src" 

C_DEPS += \
"./code/ins/ins_solver.d" \
"./code/ins/math_utils.d" 

OBJS += \
"code/ins/ins_solver.o" \
"code/ins/math_utils.o" 


# Each subdirectory must supply rules for building sources it contributes
"code/ins/ins_solver.src":"../code/ins/ins_solver.c" "code/ins/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/source/Seekfree_TC264_combined-GPS-old/Seekfree_TC264_combined-GPS-old/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/ins/ins_solver.o":"code/ins/ins_solver.src" "code/ins/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/ins/math_utils.src":"../code/ins/math_utils.c" "code/ins/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/source/Seekfree_TC264_combined-GPS-old/Seekfree_TC264_combined-GPS-old/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/ins/math_utils.o":"code/ins/math_utils.src" "code/ins/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-code-2f-ins

clean-code-2f-ins:
	-$(RM) ./code/ins/ins_solver.d ./code/ins/ins_solver.o ./code/ins/ins_solver.src ./code/ins/math_utils.d ./code/ins/math_utils.o ./code/ins/math_utils.src

.PHONY: clean-code-2f-ins

