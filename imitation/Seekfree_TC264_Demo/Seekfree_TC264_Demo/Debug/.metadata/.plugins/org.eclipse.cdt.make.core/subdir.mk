################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
CPP_SRCS += \
"../.metadata/.plugins/org.eclipse.cdt.make.core/specs.cpp" 

C_SRCS += \
"../.metadata/.plugins/org.eclipse.cdt.make.core/specs.c" 

COMPILED_SRCS += \
".metadata/.plugins/org.eclipse.cdt.make.core/specs.src" 

CPP_DEPS += \
"./.metadata/.plugins/org.eclipse.cdt.make.core/specs.d" 

C_DEPS += \
"./.metadata/.plugins/org.eclipse.cdt.make.core/specs.d" 

OBJS += \
".metadata/.plugins/org.eclipse.cdt.make.core/specs.o" 


# 每个子目录必须为构建它所贡献的源提供规则
".metadata/.plugins/org.eclipse.cdt.make.core/specs.src":"../.metadata/.plugins/org.eclipse.cdt.make.core/specs.c" ".metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Seekfree_TC264_Demo/Seekfree_TC264_Demo/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"
".metadata/.plugins/org.eclipse.cdt.make.core/specs.o":".metadata/.plugins/org.eclipse.cdt.make.core/specs.src" ".metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk"
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
".metadata/.plugins/org.eclipse.cdt.make.core/specs.src":"../.metadata/.plugins/org.eclipse.cdt.make.core/specs.cpp" ".metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk"
	cctc -cs --dep-file="$*.d" --misrac-version=2012 -D__CPU__=tc26xb "-fD:/kadingkuaipao/imitation/Seekfree_TC264_Demo/Seekfree_TC264_Demo/Debug/TASKING_C_C___Compiler-Include_paths__-I_.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -Y0 -N0 -Z0 -o "$@" "$<"

clean: clean--2e-metadata-2f--2e-plugins-2f-org-2e-eclipse-2e-cdt-2e-make-2e-core

clean--2e-metadata-2f--2e-plugins-2f-org-2e-eclipse-2e-cdt-2e-make-2e-core:
	-$(RM) ./.metadata/.plugins/org.eclipse.cdt.make.core/specs.d ./.metadata/.plugins/org.eclipse.cdt.make.core/specs.o ./.metadata/.plugins/org.eclipse.cdt.make.core/specs.src

.PHONY: clean--2e-metadata-2f--2e-plugins-2f-org-2e-eclipse-2e-cdt-2e-make-2e-core

