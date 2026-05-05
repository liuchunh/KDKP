################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../code/Steering encoder.c" \
"../code/screen.c" \
"../code/servo.c" \
"../code/zf_device_car_light.c" \
"../code/zf_device_dot_matrix_screen.c" \
"../code/zf_device_tld7002.c" 

COMPILED_SRCS += \
"code/Steering encoder.src" \
"code/screen.src" \
"code/servo.src" \
"code/zf_device_car_light.src" \
"code/zf_device_dot_matrix_screen.src" \
"code/zf_device_tld7002.src" 

C_DEPS += \
"./code/Steering encoder.d" \
"./code/screen.d" \
"./code/servo.d" \
"./code/zf_device_car_light.d" \
"./code/zf_device_dot_matrix_screen.d" \
"./code/zf_device_tld7002.d" 

OBJS += \
"code/Steering encoder.o" \
"code/screen.o" \
"code/servo.o" \
"code/zf_device_car_light.o" \
"code/zf_device_dot_matrix_screen.o" \
"code/zf_device_tld7002.o" 


# Each subdirectory must supply rules for building sources it contributes
"code/Steering encoder.src":"../code/Steering encoder.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/Steering encoder.o":"code/Steering encoder.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/screen.src":"../code/screen.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/screen.o":"code/screen.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/servo.src":"../code/servo.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/servo.o":"code/servo.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/zf_device_car_light.src":"../code/zf_device_car_light.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/zf_device_car_light.o":"code/zf_device_car_light.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/zf_device_dot_matrix_screen.src":"../code/zf_device_dot_matrix_screen.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/zf_device_dot_matrix_screen.o":"code/zf_device_dot_matrix_screen.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/zf_device_tld7002.src":"../code/zf_device_tld7002.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/ADS/KDKP4.28/Seekfree_TC264_Opensource_Library/Seekfree_TC264_general_Opensource_Library/Seekfree_TC264_Opensource_Library/Release/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O2 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/zf_device_tld7002.o":"code/zf_device_tld7002.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-code

clean-code:
	-$(RM) ./code/Steering\ encoder.d ./code/Steering\ encoder.o ./code/Steering\ encoder.src ./code/screen.d ./code/screen.o ./code/screen.src ./code/servo.d ./code/servo.o ./code/servo.src ./code/zf_device_car_light.d ./code/zf_device_car_light.o ./code/zf_device_car_light.src ./code/zf_device_dot_matrix_screen.d ./code/zf_device_dot_matrix_screen.o ./code/zf_device_dot_matrix_screen.src ./code/zf_device_tld7002.d ./code/zf_device_tld7002.o ./code/zf_device_tld7002.src

.PHONY: clean-code

