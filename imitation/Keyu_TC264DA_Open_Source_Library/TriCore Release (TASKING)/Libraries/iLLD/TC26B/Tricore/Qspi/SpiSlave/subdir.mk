################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
C_SRCS += \
"../Libraries/iLLD/TC26B/Tricore/Qspi/SpiSlave/IfxQspi_SpiSlave.c" 

COMPILED_SRCS += \
"Libraries/iLLD/TC26B/Tricore/Qspi/SpiSlave/IfxQspi_SpiSlave.src" 

C_DEPS += \
"./Libraries/iLLD/TC26B/Tricore/Qspi/SpiSlave/IfxQspi_SpiSlave.d" 

OBJS += \
"Libraries/iLLD/TC26B/Tricore/Qspi/SpiSlave/IfxQspi_SpiSlave.o" 


# 每个子目录必须为构建它所贡献的源提供规则
"Libraries/iLLD/TC26B/Tricore/Qspi/SpiSlave/IfxQspi_SpiSlave.src":"../Libraries/iLLD/TC26B/Tricore/Qspi/SpiSlave/IfxQspi_SpiSlave.c" "Libraries/iLLD/TC26B/Tricore/Qspi/SpiSlave/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Keyu_TC264DA_Open_Source_Library/TriCore Release (TASKING)/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
"Libraries/iLLD/TC26B/Tricore/Qspi/SpiSlave/IfxQspi_SpiSlave.o":"Libraries/iLLD/TC26B/Tricore/Qspi/SpiSlave/IfxQspi_SpiSlave.src" "Libraries/iLLD/TC26B/Tricore/Qspi/SpiSlave/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"

clean: clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Qspi-2f-SpiSlave

clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Qspi-2f-SpiSlave:
	-$(RM) ./Libraries/iLLD/TC26B/Tricore/Qspi/SpiSlave/IfxQspi_SpiSlave.d ./Libraries/iLLD/TC26B/Tricore/Qspi/SpiSlave/IfxQspi_SpiSlave.o ./Libraries/iLLD/TC26B/Tricore/Qspi/SpiSlave/IfxQspi_SpiSlave.src

.PHONY: clean-Libraries-2f-iLLD-2f-TC26B-2f-Tricore-2f-Qspi-2f-SpiSlave

