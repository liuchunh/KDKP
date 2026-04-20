################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
C_SRCS += \
"../keyu_library/common/ky_frame_queue.c" \
"../keyu_library/common/ky_math.c" \
"../keyu_library/common/ky_ringbuffer.c" \
"../keyu_library/common/ky_utils.c" 

COMPILED_SRCS += \
"keyu_library/common/ky_frame_queue.src" \
"keyu_library/common/ky_math.src" \
"keyu_library/common/ky_ringbuffer.src" \
"keyu_library/common/ky_utils.src" 

C_DEPS += \
"./keyu_library/common/ky_frame_queue.d" \
"./keyu_library/common/ky_math.d" \
"./keyu_library/common/ky_ringbuffer.d" \
"./keyu_library/common/ky_utils.d" 

OBJS += \
"keyu_library/common/ky_frame_queue.o" \
"keyu_library/common/ky_math.o" \
"keyu_library/common/ky_ringbuffer.o" \
"keyu_library/common/ky_utils.o" 


# 每个子目录必须为构建它所贡献的源提供规则
"keyu_library/common/ky_frame_queue.src":"../keyu_library/common/ky_frame_queue.c" "keyu_library/common/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/common/ky_frame_queue.o":"keyu_library/common/ky_frame_queue.src" "keyu_library/common/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/common/ky_math.src":"../keyu_library/common/ky_math.c" "keyu_library/common/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/common/ky_math.o":"keyu_library/common/ky_math.src" "keyu_library/common/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/common/ky_ringbuffer.src":"../keyu_library/common/ky_ringbuffer.c" "keyu_library/common/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/common/ky_ringbuffer.o":"keyu_library/common/ky_ringbuffer.src" "keyu_library/common/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"keyu_library/common/ky_utils.src":"../keyu_library/common/ky_utils.c" "keyu_library/common/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"keyu_library/common/ky_utils.o":"keyu_library/common/ky_utils.src" "keyu_library/common/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-keyu_library-2f-common

clean-keyu_library-2f-common:
	-$(RM) ./keyu_library/common/ky_frame_queue.d ./keyu_library/common/ky_frame_queue.o ./keyu_library/common/ky_frame_queue.src ./keyu_library/common/ky_math.d ./keyu_library/common/ky_math.o ./keyu_library/common/ky_math.src ./keyu_library/common/ky_ringbuffer.d ./keyu_library/common/ky_ringbuffer.o ./keyu_library/common/ky_ringbuffer.src ./keyu_library/common/ky_utils.d ./keyu_library/common/ky_utils.o ./keyu_library/common/ky_utils.src

.PHONY: clean-keyu_library-2f-common

