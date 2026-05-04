################################################################################
# 自动生成的文件。不要编辑！
################################################################################

-include ../makefile.init

RM := rm -rf

# 此处定义了参与生成的所有源
-include sources.mk
-include user/subdir.mk
-include libraries/zf_driver/subdir.mk
-include libraries/zf_device/subdir.mk
-include libraries/zf_components/subdir.mk
-include libraries/zf_common/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/_PinMap/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/_Impl/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Vadc/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Vadc/Adc/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Stm/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Src/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Scu/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Qspi/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Qspi/SpiMaster/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Port/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Mtu/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Gtm/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Gtm/Atom/Pwm/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Gpt12/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Gpt12/IncrEnc/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Flash/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Dma/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Dma/Dma/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/Irq/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Cpu/CStart/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Ccu6/Timer/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Ccu6/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Asclin/Std/subdir.mk
-include libraries/infineon_libraries/iLLD/TC26B/Tricore/Asclin/Asc/subdir.mk
-include libraries/infineon_libraries/Service/CpuGeneric/SysSe/Math/subdir.mk
-include libraries/infineon_libraries/Service/CpuGeneric/SysSe/Bsp/subdir.mk
-include libraries/infineon_libraries/Service/CpuGeneric/StdIf/subdir.mk
-include libraries/infineon_libraries/Service/CpuGeneric/If/subdir.mk
-include libraries/infineon_libraries/Infra/Platform/Tricore/Compilers/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++M_DEPS)),)
-include $(C++M_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(CCM_DEPS)),)
-include $(CCM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXXM_DEPS)),)
-include $(CXXM_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := E01_02_buzzer_demo
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# 将这些工具调用的输入和输出添加到构建变量 
SECONDARY_SIZE += \
E01_02_buzzer_demo.siz \


# 所有目标
all: main-build

# 主构建目标
main-build: E01_02_buzzer_demo.elf secondary-outputs

# 工具调用
E01_02_buzzer_demo.elf: $(OBJS) makefile $(OPTIONAL_TOOL_DEPS) .E01_02_buzzer_demo.elf.opt
	@echo '正在构建目标： $@'
	@echo '正在调用： TASKING Linker'
	cctc -f .E01_02_buzzer_demo.elf.opt
	@echo '已结束构建目标： $@'
	@echo ' '

.E01_02_buzzer_demo.elf.opt:
	@argfile .E01_02_buzzer_demo.elf.opt --lsl-file="../Lcf_Tasking_Tricore_Tc.lsl" -Wl-Oc -Wl-OL -Wl-Ot -Wl-Ox -Wl-Oy "D:\kadingkuaipao\TC264_Library-master\TC264_Library-master\Example\Example_general\Motherboard_Demo\Motherboard_Demo\E1_motherboard\E01_02_buzzer_demo\libraries\zf_device\zf_device_config.a" -Wl--map-file="E01_02_buzzer_demo.map" -Wl-mc -Wl-mf -Wl-mi -Wl-mk -Wl-ml -Wl-mm -Wl-md -Wl-mr -Wl-mu --no-warnings= -Wl--error-limit=42 --fp-model=3 -lrt --lsl-core=vtc --exceptions --strict --anachronisms --force-c++ -Ctc26xb -o"E01_02_buzzer_demo.elf" -Wl-o"E01_02_buzzer_demo.hex:IHEX" $(OBJS)

.PHONY: .E01_02_buzzer_demo.elf.opt

E01_02_buzzer_demo.siz: E01_02_buzzer_demo.elf makefile $(OPTIONAL_TOOL_DEPS)
	@echo '正在调用： Print Size'
	elfsize  "E01_02_buzzer_demo.elf"
	@echo '已结束构建： $@'
	@echo ' '

# 其他目标
clean:
	-$(RM) E01_02_buzzer_demo.siz E01_02_buzzer_demo.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_SIZE)

.PHONY: all clean dependents main-build

-include ../makefile.targets
