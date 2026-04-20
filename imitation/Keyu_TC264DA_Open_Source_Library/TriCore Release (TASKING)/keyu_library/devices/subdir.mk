################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
C_SRCS += \
"../keyu_library/devices/ky_config.c" \
"../keyu_library/devices/ky_font.c" \
"../keyu_library/devices/ky_gnss.c" \
"../keyu_library/devices/ky_icm45686.c" \
"../keyu_library/devices/ky_imu_base.c" \
"../keyu_library/devices/ky_ips.c" \
"../keyu_library/devices/ky_key.c" \
"../keyu_library/devices/ky_lsm6der.c" \
"../keyu_library/devices/ky_mt9v034.c" \
"../keyu_library/devices/ky_tau1202.c" \
"../keyu_library/devices/ky_vi5300.c" 

COMPILED_SRCS += \
"keyu_library/devices/ky_config.src" \
"keyu_library/devices/ky_font.src" \
"keyu_library/devices/ky_gnss.src" \
"keyu_library/devices/ky_icm45686.src" \
"keyu_library/devices/ky_imu_base.src" \
"keyu_library/devices/ky_ips.src" \
"keyu_library/devices/ky_key.src" \
"keyu_library/devices/ky_lsm6der.src" \
"keyu_library/devices/ky_mt9v034.src" \
"keyu_library/devices/ky_tau1202.src" \
"keyu_library/devices/ky_vi5300.src" 

C_DEPS += \
"./keyu_library/devices/ky_config.d" \
"./keyu_library/devices/ky_font.d" \
"./keyu_library/devices/ky_gnss.d" \
"./keyu_library/devices/ky_icm45686.d" \
"./keyu_library/devices/ky_imu_base.d" \
"./keyu_library/devices/ky_ips.d" \
"./keyu_library/devices/ky_key.d" \
"./keyu_library/devices/ky_lsm6der.d" \
"./keyu_library/devices/ky_mt9v034.d" \
"./keyu_library/devices/ky_tau1202.d" \
"./keyu_library/devices/ky_vi5300.d" 

OBJS += \
"keyu_library/devices/ky_config.o" \
"keyu_library/devices/ky_font.o" \
"keyu_library/devices/ky_gnss.o" \
"keyu_library/devices/ky_icm45686.o" \
"keyu_library/devices/ky_imu_base.o" \
"keyu_library/devices/ky_ips.o" \
"keyu_library/devices/ky_key.o" \
"keyu_library/devices/ky_lsm6der.o" \
"keyu_library/devices/ky_mt9v034.o" \
"keyu_library/devices/ky_tau1202.o" \
"keyu_library/devices/ky_vi5300.o" 


# 每个子目录必须为构建它所贡献的源提供规则
"keyu_library/devices/ky_config.src":"../keyu_library/devices/ky_config.c" "keyu_library/devices/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/devices/ky_config.o":"keyu_library/devices/ky_config.src" "keyu_library/devices/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/devices/ky_font.src":"../keyu_library/devices/ky_font.c" "keyu_library/devices/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/devices/ky_font.o":"keyu_library/devices/ky_font.src" "keyu_library/devices/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/devices/ky_gnss.src":"../keyu_library/devices/ky_gnss.c" "keyu_library/devices/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/devices/ky_gnss.o":"keyu_library/devices/ky_gnss.src" "keyu_library/devices/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/devices/ky_icm45686.src":"../keyu_library/devices/ky_icm45686.c" "keyu_library/devices/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/devices/ky_icm45686.o":"keyu_library/devices/ky_icm45686.src" "keyu_library/devices/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/devices/ky_imu_base.src":"../keyu_library/devices/ky_imu_base.c" "keyu_library/devices/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/devices/ky_imu_base.o":"keyu_library/devices/ky_imu_base.src" "keyu_library/devices/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/devices/ky_ips.src":"../keyu_library/devices/ky_ips.c" "keyu_library/devices/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/devices/ky_ips.o":"keyu_library/devices/ky_ips.src" "keyu_library/devices/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/devices/ky_key.src":"../keyu_library/devices/ky_key.c" "keyu_library/devices/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/devices/ky_key.o":"keyu_library/devices/ky_key.src" "keyu_library/devices/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/devices/ky_lsm6der.src":"../keyu_library/devices/ky_lsm6der.c" "keyu_library/devices/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/devices/ky_lsm6der.o":"keyu_library/devices/ky_lsm6der.src" "keyu_library/devices/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/devices/ky_mt9v034.src":"../keyu_library/devices/ky_mt9v034.c" "keyu_library/devices/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/devices/ky_mt9v034.o":"keyu_library/devices/ky_mt9v034.src" "keyu_library/devices/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/devices/ky_tau1202.src":"../keyu_library/devices/ky_tau1202.c" "keyu_library/devices/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/devices/ky_tau1202.o":"keyu_library/devices/ky_tau1202.src" "keyu_library/devices/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/devices/ky_vi5300.src":"../keyu_library/devices/ky_vi5300.c" "keyu_library/devices/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/devices/ky_vi5300.o":"keyu_library/devices/ky_vi5300.src" "keyu_library/devices/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-keyu_library-2f-devices

clean-keyu_library-2f-devices:
	-$(RM) ./keyu_library/devices/ky_config.d ./keyu_library/devices/ky_config.o ./keyu_library/devices/ky_config.src ./keyu_library/devices/ky_font.d ./keyu_library/devices/ky_font.o ./keyu_library/devices/ky_font.src ./keyu_library/devices/ky_gnss.d ./keyu_library/devices/ky_gnss.o ./keyu_library/devices/ky_gnss.src ./keyu_library/devices/ky_icm45686.d ./keyu_library/devices/ky_icm45686.o ./keyu_library/devices/ky_icm45686.src ./keyu_library/devices/ky_imu_base.d ./keyu_library/devices/ky_imu_base.o ./keyu_library/devices/ky_imu_base.src ./keyu_library/devices/ky_ips.d ./keyu_library/devices/ky_ips.o ./keyu_library/devices/ky_ips.src ./keyu_library/devices/ky_key.d ./keyu_library/devices/ky_key.o ./keyu_library/devices/ky_key.src ./keyu_library/devices/ky_lsm6der.d ./keyu_library/devices/ky_lsm6der.o ./keyu_library/devices/ky_lsm6der.src ./keyu_library/devices/ky_mt9v034.d ./keyu_library/devices/ky_mt9v034.o ./keyu_library/devices/ky_mt9v034.src ./keyu_library/devices/ky_tau1202.d ./keyu_library/devices/ky_tau1202.o ./keyu_library/devices/ky_tau1202.src ./keyu_library/devices/ky_vi5300.d ./keyu_library/devices/ky_vi5300.o ./keyu_library/devices/ky_vi5300.src

.PHONY: clean-keyu_library-2f-devices

