################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
C_SRCS += \
"../PID_Export/encoder.c" \
"../PID_Export/example_main.c" \
"../PID_Export/motor.c" \
"../PID_Export/motor_pid.c" 

COMPILED_SRCS += \
"PID_Export/encoder.src" \
"PID_Export/example_main.src" \
"PID_Export/motor.src" \
"PID_Export/motor_pid.src" 

C_DEPS += \
"./PID_Export/encoder.d" \
"./PID_Export/example_main.d" \
"./PID_Export/motor.d" \
"./PID_Export/motor_pid.d" 

OBJS += \
"PID_Export/encoder.o" \
"PID_Export/example_main.o" \
"PID_Export/motor.o" \
"PID_Export/motor_pid.o" 


# 每个子目录必须为构建它所贡献的源提供规则
"PID_Export/encoder.src":"../PID_Export/encoder.c" "PID_Export/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/imu/Seekfree_TC264_Opensource_Library/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"PID_Export/encoder.o":"PID_Export/encoder.src" "PID_Export/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"PID_Export/example_main.src":"../PID_Export/example_main.c" "PID_Export/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/imu/Seekfree_TC264_Opensource_Library/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"PID_Export/example_main.o":"PID_Export/example_main.src" "PID_Export/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"PID_Export/motor.src":"../PID_Export/motor.c" "PID_Export/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/imu/Seekfree_TC264_Opensource_Library/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"PID_Export/motor.o":"PID_Export/motor.src" "PID_Export/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"PID_Export/motor_pid.src":"../PID_Export/motor_pid.c" "PID_Export/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/imu/Seekfree_TC264_Opensource_Library/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"PID_Export/motor_pid.o":"PID_Export/motor_pid.src" "PID_Export/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-PID_Export

clean-PID_Export:
	-$(RM) ./PID_Export/encoder.d ./PID_Export/encoder.o ./PID_Export/encoder.src ./PID_Export/example_main.d ./PID_Export/example_main.o ./PID_Export/example_main.src ./PID_Export/motor.d ./PID_Export/motor.o ./PID_Export/motor.src ./PID_Export/motor_pid.d ./PID_Export/motor_pid.o ./PID_Export/motor_pid.src

.PHONY: clean-PID_Export

