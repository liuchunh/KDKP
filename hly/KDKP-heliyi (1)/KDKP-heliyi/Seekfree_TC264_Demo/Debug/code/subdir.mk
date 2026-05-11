################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../code/ComOutput.c" \
"../code/PID.c" \
"../code/angle_control.c" \
"../code/motor.c" \
"../code/motor_pid.c" \
"../code/setPoint.c" 

COMPILED_SRCS += \
"code/ComOutput.src" \
"code/PID.src" \
"code/angle_control.src" \
"code/motor.src" \
"code/motor_pid.src" \
"code/setPoint.src" 

C_DEPS += \
"./code/ComOutput.d" \
"./code/PID.d" \
"./code/angle_control.d" \
"./code/motor.d" \
"./code/motor_pid.d" \
"./code/setPoint.d" 

OBJS += \
"code/ComOutput.o" \
"code/PID.o" \
"code/angle_control.o" \
"code/motor.o" \
"code/motor_pid.o" \
"code/setPoint.o" 


# Each subdirectory must supply rules for building sources it contributes
"code/ComOutput.src":"../code/ComOutput.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/hly/KDKP-heliyi (1)/KDKP-heliyi/Seekfree_TC264_Demo/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/ComOutput.o":"code/ComOutput.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/PID.src":"../code/PID.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/hly/KDKP-heliyi (1)/KDKP-heliyi/Seekfree_TC264_Demo/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/PID.o":"code/PID.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/angle_control.src":"../code/angle_control.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/hly/KDKP-heliyi (1)/KDKP-heliyi/Seekfree_TC264_Demo/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/angle_control.o":"code/angle_control.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/motor.src":"../code/motor.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/hly/KDKP-heliyi (1)/KDKP-heliyi/Seekfree_TC264_Demo/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/motor.o":"code/motor.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/motor_pid.src":"../code/motor_pid.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/hly/KDKP-heliyi (1)/KDKP-heliyi/Seekfree_TC264_Demo/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/motor_pid.o":"code/motor_pid.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/setPoint.src":"../code/setPoint.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/hly/KDKP-heliyi (1)/KDKP-heliyi/Seekfree_TC264_Demo/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/setPoint.o":"code/setPoint.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-code

clean-code:
	-$(RM) ./code/ComOutput.d ./code/ComOutput.o ./code/ComOutput.src ./code/PID.d ./code/PID.o ./code/PID.src ./code/angle_control.d ./code/angle_control.o ./code/angle_control.src ./code/motor.d ./code/motor.o ./code/motor.src ./code/motor_pid.d ./code/motor_pid.o ./code/motor_pid.src ./code/setPoint.d ./code/setPoint.o ./code/setPoint.src

.PHONY: clean-code

