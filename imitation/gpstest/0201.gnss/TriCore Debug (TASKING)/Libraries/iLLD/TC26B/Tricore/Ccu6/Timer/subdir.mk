################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
C_SRCS += \
"../Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/IfxCcu6_Timer.c" 

COMPILED_SRCS += \
"Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/IfxCcu6_Timer.src" 

C_DEPS += \
"./Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/IfxCcu6_Timer.d" 

OBJS += \
"Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/IfxCcu6_Timer.o" 


# 每个子目录必须为构建它所贡献的源提供规则
"Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/IfxCcu6_Timer.src":"../Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/IfxCcu6_Timer.c" "Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/0201.gnss/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/IfxCcu6_Timer.o":"Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/IfxCcu6_Timer.src" "Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Ccu6-2f-Timer

clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Ccu6-2f-Timer:
	-$(RM) ./Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/IfxCcu6_Timer.d ./Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/IfxCcu6_Timer.o ./Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/IfxCcu6_Timer.src

.PHONY: clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Ccu6-2f-Timer

