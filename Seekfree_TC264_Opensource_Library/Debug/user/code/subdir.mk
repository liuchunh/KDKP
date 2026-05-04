################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../user/code/fusion_task.c" \
"../user/code/imu_task.c" \
"../user/code/ins_solver.c" \
"../user/code/kalman_filter.c" \
"../user/code/math_utils.c" \
"../user/code/nav_controller.c" \
"../user/code/pid_controller.c" 

COMPILED_SRCS += \
"user/code/fusion_task.src" \
"user/code/imu_task.src" \
"user/code/ins_solver.src" \
"user/code/kalman_filter.src" \
"user/code/math_utils.src" \
"user/code/nav_controller.src" \
"user/code/pid_controller.src" 

C_DEPS += \
"./user/code/fusion_task.d" \
"./user/code/imu_task.d" \
"./user/code/ins_solver.d" \
"./user/code/kalman_filter.d" \
"./user/code/math_utils.d" \
"./user/code/nav_controller.d" \
"./user/code/pid_controller.d" 

OBJS += \
"user/code/fusion_task.o" \
"user/code/imu_task.o" \
"user/code/ins_solver.o" \
"user/code/kalman_filter.o" \
"user/code/math_utils.o" \
"user/code/nav_controller.o" \
"user/code/pid_controller.o" 


# Each subdirectory must supply rules for building sources it contributes
"user/code/fusion_task.src":"../user/code/fusion_task.c" "user/code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/tools/Infineon/ADS_Workspace/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"user/code/fusion_task.o":"user/code/fusion_task.src" "user/code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"user/code/imu_task.src":"../user/code/imu_task.c" "user/code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/tools/Infineon/ADS_Workspace/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"user/code/imu_task.o":"user/code/imu_task.src" "user/code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"user/code/ins_solver.src":"../user/code/ins_solver.c" "user/code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/tools/Infineon/ADS_Workspace/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"user/code/ins_solver.o":"user/code/ins_solver.src" "user/code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"user/code/kalman_filter.src":"../user/code/kalman_filter.c" "user/code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/tools/Infineon/ADS_Workspace/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"user/code/kalman_filter.o":"user/code/kalman_filter.src" "user/code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"user/code/math_utils.src":"../user/code/math_utils.c" "user/code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/tools/Infineon/ADS_Workspace/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"user/code/math_utils.o":"user/code/math_utils.src" "user/code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"user/code/nav_controller.src":"../user/code/nav_controller.c" "user/code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/tools/Infineon/ADS_Workspace/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"user/code/nav_controller.o":"user/code/nav_controller.src" "user/code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"user/code/pid_controller.src":"../user/code/pid_controller.c" "user/code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/tools/Infineon/ADS_Workspace/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"user/code/pid_controller.o":"user/code/pid_controller.src" "user/code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-user-2f-code

clean-user-2f-code:
	-$(RM) ./user/code/fusion_task.d ./user/code/fusion_task.o ./user/code/fusion_task.src ./user/code/imu_task.d ./user/code/imu_task.o ./user/code/imu_task.src ./user/code/ins_solver.d ./user/code/ins_solver.o ./user/code/ins_solver.src ./user/code/kalman_filter.d ./user/code/kalman_filter.o ./user/code/kalman_filter.src ./user/code/math_utils.d ./user/code/math_utils.o ./user/code/math_utils.src ./user/code/nav_controller.d ./user/code/nav_controller.o ./user/code/nav_controller.src ./user/code/pid_controller.d ./user/code/pid_controller.o ./user/code/pid_controller.src

.PHONY: clean-user-2f-code

