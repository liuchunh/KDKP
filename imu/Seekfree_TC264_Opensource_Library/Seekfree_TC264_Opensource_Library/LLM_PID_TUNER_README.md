# LLM-PID-Tuner 集成说明

## 概述

本项目已集成 LLM-PID-Tuner 工具，可以通过大语言模型自动调优速度 PID 参数。

## 工作原理

```
TC264 (下位机) --串口CSV--> tuner.py (上位机) --API--> LLM
                              |
                              v
                     LLM 返回新 PID 参数
                              |
                              v
TC264 <--串口指令-- tuner.py
```

## 串口协议

### TC264 上报格式 (CSV)
```
timestamp_ms,setpoint,input,pwm,error,p,i,d
```
示例：`5000,1.50,1.23,3500,0.27,3.00,0.50,0.10`

- `timestamp_ms`: 系统时间 (ms)
- `setpoint`: 目标速度 (m/s)
- `input`: 编码器实测速度 (m/s)
- `pwm`: PWM 占空比
- `error`: 速度误差
- `p,i,d`: 当前 PID 参数

### 上位机下发指令
```
SET P:1.5 I:0.2 D:0.05
PID 1.5 0.2 0.05
STATUS
```

## 使用步骤

### 1. 准备工作

1. 安装 LLM-PID-Tuner：
   ```bash
   cd D:\kadingkuaipao\llm-pid-tuner-dev\llm-pid-tuner-dev
   pip install -r requirements.txt
   ```

2. 配置 `config.json`：
   ```json
   {
     "SERIAL_PORT": "AUTO",
     "BAUD_RATE": 115200,
     "LLM_API_KEY": "你的API密钥",
     "LLM_API_BASE_URL": "https://api.openai.com/v1",
     "LLM_MODEL_NAME": "gpt-4o",
     "LLM_PROVIDER": "openai"
   }
   ```

### 2. 烧录固件

1. 编译并烧录 `cpu0_main.c` 到 TC264
2. 确保电机和编码器正常工作

### 3. 运行调参

1. 连接 TC264 到电脑 (USB转串口)
2. 运行 LLM-PID-Tuner：
   ```bash
   python tuner.py
   ```
3. 选择串口端口
4. 等待 LLM 分析并给出 PID 参数建议

### 4. 测试调参结果

1. LLM 会自动下发新的 PID 参数
2. 观察串口输出，确认参数已更新
3. 测试车模运行效果
4. 如果效果不好，LLM 会自动回退到之前的参数

## 参数说明

### 速度 PID 参数 (增量式)

| 参数 | 说明 | 初始值 | 范围 |
|------|------|--------|------|
| Kp | 比例系数 | 3.0 | 0-100 |
| Ki | 积分系数 | 0.5 | 0-50 |
| Kd | 微分系数 | 0.1 | 0-50 |

### 编码器参数

| 参数 | 说明 | 值 |
|------|------|-----|
| WHEEL_DIAMETER | 轮子直径 (m) | 0.065 |
| GEAR_RATIO | 减速比 | 30.0 |

**注意**：请根据实际车模调整这些参数！

## 调试命令

### 查看当前 PID 参数
发送：`STATUS`

### 手动设置 PID 参数
发送：`SET P:3.0 I:0.5 D:0.1`

或：`PID 3.0 0.5 0.1`

## 故障排除

### 1. 串口无数据
- 检查串口连接
- 确认波特率 115200
- 检查 TC264 是否正常运行

### 2. LLM 无法调参
- 检查 API 密钥是否正确
- 确认网络连接正常
- 查看 LLM-PID-Tuner 的日志输出

### 3. 编码器速度异常
- 检查编码器接线
- 确认轮子直径和减速比设置正确
- 使用 `STATUS` 命令查看当前速度

## 注意事项

1. **安全第一**：调参时请有人值守，准备紧急停车
2. **参数范围**：PID 参数有范围限制，超出会被拒绝
3. **自动回退**：如果调参效果变差，LLM 会自动回退到之前的参数
4. **积分清零**：每次更新 PID 参数后，积分项会自动清零

## 文件说明

- `cpu0_main.c`: 主程序，包含 LLM-PID-Tuner 集成代码
- `config.json`: LLM-PID-Tuner 配置文件
- `pid_controller.c`: PID 控制器实现 (支持增量式)
- `encoder.c`: 编码器驱动

## 联系方式

如有问题，请查看：
- LLM-PID-Tuner 项目：https://github.com/KINGSTON-115/llm-pid-tuner
- QQ 群：1082281492
