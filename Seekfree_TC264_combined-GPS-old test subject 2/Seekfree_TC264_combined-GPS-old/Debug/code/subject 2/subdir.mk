################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../code/subject 2/action_task.c" 

COMPILED_SRCS += \
"code/subject 2/action_task.src" 

C_DEPS += \
"./code/subject 2/action_task.d" 

OBJS += \
"code/subject 2/action_task.o" 


# Each subdirectory must supply rules for building sources it contributes
"code/subject 2/action_task.src":"../code/subject 2/action_task.c" "code/subject 2/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/source/Seekfree_TC264_combined-GPS-old test subject 2/Seekfree_TC264_combined-GPS-old/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/subject 2/action_task.o":"code/subject 2/action_task.src" "code/subject 2/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-code-2f-subject-20-2

clean-code-2f-subject-20-2:
	-$(RM) ./code/subject\ 2/action_task.d ./code/subject\ 2/action_task.o ./code/subject\ 2/action_task.src

.PHONY: clean-code-2f-subject-20-2

