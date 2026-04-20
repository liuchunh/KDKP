################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../code/keyu_PID.c" \
"../code/keyu_all_parameter.c" \
"../code/keyu_control_car.c" \
"../code/keyu_filter.c" \
"../code/keyu_imu.c" \
"../code/keyu_init.c" \
"../code/keyu_motor.c" \
"../code/keyu_use_encoder.c" 

COMPILED_SRCS += \
"code/keyu_PID.src" \
"code/keyu_all_parameter.src" \
"code/keyu_control_car.src" \
"code/keyu_filter.src" \
"code/keyu_imu.src" \
"code/keyu_init.src" \
"code/keyu_motor.src" \
"code/keyu_use_encoder.src" 

C_DEPS += \
"./code/keyu_PID.d" \
"./code/keyu_all_parameter.d" \
"./code/keyu_control_car.d" \
"./code/keyu_filter.d" \
"./code/keyu_imu.d" \
"./code/keyu_init.d" \
"./code/keyu_motor.d" \
"./code/keyu_use_encoder.d" 

OBJS += \
"code/keyu_PID.o" \
"code/keyu_all_parameter.o" \
"code/keyu_control_car.o" \
"code/keyu_filter.o" \
"code/keyu_imu.o" \
"code/keyu_init.o" \
"code/keyu_motor.o" \
"code/keyu_use_encoder.o" 


# Each subdirectory must supply rules for building sources it contributes
"code/keyu_PID.src":"../code/keyu_PID.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fE:/software/AURIX-v1.10.28-workspace/Keyu_TC264DA_Open_Source_Library-master/Keyu_TC264DA_Open_Source_Library/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/keyu_PID.o":"code/keyu_PID.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/keyu_all_parameter.src":"../code/keyu_all_parameter.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fE:/software/AURIX-v1.10.28-workspace/Keyu_TC264DA_Open_Source_Library-master/Keyu_TC264DA_Open_Source_Library/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/keyu_all_parameter.o":"code/keyu_all_parameter.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/keyu_control_car.src":"../code/keyu_control_car.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fE:/software/AURIX-v1.10.28-workspace/Keyu_TC264DA_Open_Source_Library-master/Keyu_TC264DA_Open_Source_Library/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/keyu_control_car.o":"code/keyu_control_car.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/keyu_filter.src":"../code/keyu_filter.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fE:/software/AURIX-v1.10.28-workspace/Keyu_TC264DA_Open_Source_Library-master/Keyu_TC264DA_Open_Source_Library/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/keyu_filter.o":"code/keyu_filter.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/keyu_imu.src":"../code/keyu_imu.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fE:/software/AURIX-v1.10.28-workspace/Keyu_TC264DA_Open_Source_Library-master/Keyu_TC264DA_Open_Source_Library/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/keyu_imu.o":"code/keyu_imu.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/keyu_init.src":"../code/keyu_init.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fE:/software/AURIX-v1.10.28-workspace/Keyu_TC264DA_Open_Source_Library-master/Keyu_TC264DA_Open_Source_Library/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/keyu_init.o":"code/keyu_init.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/keyu_motor.src":"../code/keyu_motor.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fE:/software/AURIX-v1.10.28-workspace/Keyu_TC264DA_Open_Source_Library-master/Keyu_TC264DA_Open_Source_Library/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/keyu_motor.o":"code/keyu_motor.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/keyu_use_encoder.src":"../code/keyu_use_encoder.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fE:/software/AURIX-v1.10.28-workspace/Keyu_TC264DA_Open_Source_Library-master/Keyu_TC264DA_Open_Source_Library/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/keyu_use_encoder.o":"code/keyu_use_encoder.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-code

clean-code:
	-$(RM) ./code/keyu_PID.d ./code/keyu_PID.o ./code/keyu_PID.src ./code/keyu_all_parameter.d ./code/keyu_all_parameter.o ./code/keyu_all_parameter.src ./code/keyu_control_car.d ./code/keyu_control_car.o ./code/keyu_control_car.src ./code/keyu_filter.d ./code/keyu_filter.o ./code/keyu_filter.src ./code/keyu_imu.d ./code/keyu_imu.o ./code/keyu_imu.src ./code/keyu_init.d ./code/keyu_init.o ./code/keyu_init.src ./code/keyu_motor.d ./code/keyu_motor.o ./code/keyu_motor.src ./code/keyu_use_encoder.d ./code/keyu_use_encoder.o ./code/keyu_use_encoder.src

.PHONY: clean-code

