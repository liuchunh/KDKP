################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
C_SRCS += \
"../code/ComOutput.c" \
"../code/setPoint.c" 

COMPILED_SRCS += \
"code/ComOutput.src" \
"code/setPoint.src" 

C_DEPS += \
"./code/ComOutput.d" \
"./code/setPoint.d" 

OBJS += \
"code/ComOutput.o" \
"code/setPoint.o" 


# 每个子目录必须为构建它所贡献的源提供规则
"code/ComOutput.src":"../code/ComOutput.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Seekfree/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/ComOutput.o":"code/ComOutput.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
"code/setPoint.src":"../code/setPoint.c" "code/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Seekfree/Seekfree_TC264_Opensource_Library/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"code/setPoint.o":"code/setPoint.src" "code/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-code

clean-code:
	-$(RM) ./code/ComOutput.d ./code/ComOutput.o ./code/ComOutput.src ./code/setPoint.d ./code/setPoint.o ./code/setPoint.src

.PHONY: clean-code

