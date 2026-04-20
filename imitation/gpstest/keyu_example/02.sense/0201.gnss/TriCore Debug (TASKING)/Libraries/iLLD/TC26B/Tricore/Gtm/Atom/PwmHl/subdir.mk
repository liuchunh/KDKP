################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
C_SRCS += \
"../Libraries/iLLD/TC26B/Tricore/Gtm/Atom/PwmHl/IfxGtm_Atom_PwmHl.c" 

COMPILED_SRCS += \
"Libraries/iLLD/TC26B/Tricore/Gtm/Atom/PwmHl/IfxGtm_Atom_PwmHl.src" 

C_DEPS += \
"./Libraries/iLLD/TC26B/Tricore/Gtm/Atom/PwmHl/IfxGtm_Atom_PwmHl.d" 

OBJS += \
"Libraries/iLLD/TC26B/Tricore/Gtm/Atom/PwmHl/IfxGtm_Atom_PwmHl.o" 


# 每个子目录必须为构建它所贡献的源提供规则
"Libraries/iLLD/TC26B/Tricore/Gtm/Atom/PwmHl/IfxGtm_Atom_PwmHl.src":"../Libraries/iLLD/TC26B/Tricore/Gtm/Atom/PwmHl/IfxGtm_Atom_PwmHl.c" "Libraries/iLLD/TC26B/Tricore/Gtm/Atom/PwmHl/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/gpstest/keyu_example/02.sense/0201.gnss/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"Libraries/iLLD/TC26B/Tricore/Gtm/Atom/PwmHl/IfxGtm_Atom_PwmHl.o":"Libraries/iLLD/TC26B/Tricore/Gtm/Atom/PwmHl/IfxGtm_Atom_PwmHl.src" "Libraries/iLLD/TC26B/Tricore/Gtm/Atom/PwmHl/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Gtm-2f-Atom-2f-PwmHl

clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Gtm-2f-Atom-2f-PwmHl:
	-$(RM) ./Libraries/iLLD/TC26B/Tricore/Gtm/Atom/PwmHl/IfxGtm_Atom_PwmHl.d ./Libraries/iLLD/TC26B/Tricore/Gtm/Atom/PwmHl/IfxGtm_Atom_PwmHl.o ./Libraries/iLLD/TC26B/Tricore/Gtm/Atom/PwmHl/IfxGtm_Atom_PwmHl.src

.PHONY: clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Gtm-2f-Atom-2f-PwmHl

