################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
"../keyu_library/drivers/ky_adc.c" \
"../keyu_library/drivers/ky_delay.c" \
"../keyu_library/drivers/ky_dma.c" \
"../keyu_library/drivers/ky_eeprom.c" \
"../keyu_library/drivers/ky_encoder.c" \
"../keyu_library/drivers/ky_exti.c" \
"../keyu_library/drivers/ky_gpio.c" \
"../keyu_library/drivers/ky_i2c.c" \
"../keyu_library/drivers/ky_pwm.c" \
"../keyu_library/drivers/ky_soft_i2c.c" \
"../keyu_library/drivers/ky_spi.c" \
"../keyu_library/drivers/ky_timer.c" \
"../keyu_library/drivers/ky_uart.c" 

COMPILED_SRCS += \
"keyu_library/drivers/ky_adc.src" \
"keyu_library/drivers/ky_delay.src" \
"keyu_library/drivers/ky_dma.src" \
"keyu_library/drivers/ky_eeprom.src" \
"keyu_library/drivers/ky_encoder.src" \
"keyu_library/drivers/ky_exti.src" \
"keyu_library/drivers/ky_gpio.src" \
"keyu_library/drivers/ky_i2c.src" \
"keyu_library/drivers/ky_pwm.src" \
"keyu_library/drivers/ky_soft_i2c.src" \
"keyu_library/drivers/ky_spi.src" \
"keyu_library/drivers/ky_timer.src" \
"keyu_library/drivers/ky_uart.src" 

C_DEPS += \
"./keyu_library/drivers/ky_adc.d" \
"./keyu_library/drivers/ky_delay.d" \
"./keyu_library/drivers/ky_dma.d" \
"./keyu_library/drivers/ky_eeprom.d" \
"./keyu_library/drivers/ky_encoder.d" \
"./keyu_library/drivers/ky_exti.d" \
"./keyu_library/drivers/ky_gpio.d" \
"./keyu_library/drivers/ky_i2c.d" \
"./keyu_library/drivers/ky_pwm.d" \
"./keyu_library/drivers/ky_soft_i2c.d" \
"./keyu_library/drivers/ky_spi.d" \
"./keyu_library/drivers/ky_timer.d" \
"./keyu_library/drivers/ky_uart.d" 

OBJS += \
"keyu_library/drivers/ky_adc.o" \
"keyu_library/drivers/ky_delay.o" \
"keyu_library/drivers/ky_dma.o" \
"keyu_library/drivers/ky_eeprom.o" \
"keyu_library/drivers/ky_encoder.o" \
"keyu_library/drivers/ky_exti.o" \
"keyu_library/drivers/ky_gpio.o" \
"keyu_library/drivers/ky_i2c.o" \
"keyu_library/drivers/ky_pwm.o" \
"keyu_library/drivers/ky_soft_i2c.o" \
"keyu_library/drivers/ky_spi.o" \
"keyu_library/drivers/ky_timer.o" \
"keyu_library/drivers/ky_uart.o" 


# Each subdirectory must supply rules for building sources it contributes
"keyu_library/drivers/ky_adc.src":"../keyu_library/drivers/ky_adc.c" "keyu_library/drivers/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/drivers/ky_adc.o":"keyu_library/drivers/ky_adc.src" "keyu_library/drivers/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/drivers/ky_delay.src":"../keyu_library/drivers/ky_delay.c" "keyu_library/drivers/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/drivers/ky_delay.o":"keyu_library/drivers/ky_delay.src" "keyu_library/drivers/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/drivers/ky_dma.src":"../keyu_library/drivers/ky_dma.c" "keyu_library/drivers/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/drivers/ky_dma.o":"keyu_library/drivers/ky_dma.src" "keyu_library/drivers/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/drivers/ky_eeprom.src":"../keyu_library/drivers/ky_eeprom.c" "keyu_library/drivers/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/drivers/ky_eeprom.o":"keyu_library/drivers/ky_eeprom.src" "keyu_library/drivers/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/drivers/ky_encoder.src":"../keyu_library/drivers/ky_encoder.c" "keyu_library/drivers/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/drivers/ky_encoder.o":"keyu_library/drivers/ky_encoder.src" "keyu_library/drivers/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/drivers/ky_exti.src":"../keyu_library/drivers/ky_exti.c" "keyu_library/drivers/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/drivers/ky_exti.o":"keyu_library/drivers/ky_exti.src" "keyu_library/drivers/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/drivers/ky_gpio.src":"../keyu_library/drivers/ky_gpio.c" "keyu_library/drivers/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/drivers/ky_gpio.o":"keyu_library/drivers/ky_gpio.src" "keyu_library/drivers/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/drivers/ky_i2c.src":"../keyu_library/drivers/ky_i2c.c" "keyu_library/drivers/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/drivers/ky_i2c.o":"keyu_library/drivers/ky_i2c.src" "keyu_library/drivers/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/drivers/ky_pwm.src":"../keyu_library/drivers/ky_pwm.c" "keyu_library/drivers/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/drivers/ky_pwm.o":"keyu_library/drivers/ky_pwm.src" "keyu_library/drivers/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/drivers/ky_soft_i2c.src":"../keyu_library/drivers/ky_soft_i2c.c" "keyu_library/drivers/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/drivers/ky_soft_i2c.o":"keyu_library/drivers/ky_soft_i2c.src" "keyu_library/drivers/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/drivers/ky_spi.src":"../keyu_library/drivers/ky_spi.c" "keyu_library/drivers/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/drivers/ky_spi.o":"keyu_library/drivers/ky_spi.src" "keyu_library/drivers/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/drivers/ky_timer.src":"../keyu_library/drivers/ky_timer.c" "keyu_library/drivers/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/drivers/ky_timer.o":"keyu_library/drivers/ky_timer.src" "keyu_library/drivers/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/drivers/ky_uart.src":"../keyu_library/drivers/ky_uart.c" "keyu_library/drivers/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0204.ips200/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/drivers/ky_uart.o":"keyu_library/drivers/ky_uart.src" "keyu_library/drivers/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-keyu_library-2f-drivers

clean-keyu_library-2f-drivers:
	-$(RM) ./keyu_library/drivers/ky_adc.d ./keyu_library/drivers/ky_adc.o ./keyu_library/drivers/ky_adc.src ./keyu_library/drivers/ky_delay.d ./keyu_library/drivers/ky_delay.o ./keyu_library/drivers/ky_delay.src ./keyu_library/drivers/ky_dma.d ./keyu_library/drivers/ky_dma.o ./keyu_library/drivers/ky_dma.src ./keyu_library/drivers/ky_eeprom.d ./keyu_library/drivers/ky_eeprom.o ./keyu_library/drivers/ky_eeprom.src ./keyu_library/drivers/ky_encoder.d ./keyu_library/drivers/ky_encoder.o ./keyu_library/drivers/ky_encoder.src ./keyu_library/drivers/ky_exti.d ./keyu_library/drivers/ky_exti.o ./keyu_library/drivers/ky_exti.src ./keyu_library/drivers/ky_gpio.d ./keyu_library/drivers/ky_gpio.o ./keyu_library/drivers/ky_gpio.src ./keyu_library/drivers/ky_i2c.d ./keyu_library/drivers/ky_i2c.o ./keyu_library/drivers/ky_i2c.src ./keyu_library/drivers/ky_pwm.d ./keyu_library/drivers/ky_pwm.o ./keyu_library/drivers/ky_pwm.src ./keyu_library/drivers/ky_soft_i2c.d ./keyu_library/drivers/ky_soft_i2c.o ./keyu_library/drivers/ky_soft_i2c.src ./keyu_library/drivers/ky_spi.d ./keyu_library/drivers/ky_spi.o ./keyu_library/drivers/ky_spi.src ./keyu_library/drivers/ky_timer.d ./keyu_library/drivers/ky_timer.o ./keyu_library/drivers/ky_timer.src ./keyu_library/drivers/ky_uart.d ./keyu_library/drivers/ky_uart.o ./keyu_library/drivers/ky_uart.src

.PHONY: clean-keyu_library-2f-drivers

