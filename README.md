# STM32G474 三相逆变控制工程

本仓库保存基于 STM32G474RBT6 的三相逆变控制固件，包含可在 VSCode 中编辑的源码与配置，以及可直接在 Keil MDK 中打开的工程文件。

## 目录说明

- `Core/`：STM32CubeMX 生成的启动和外设初始化代码
- `Drivers/`：STM32 HAL、CMSIS 与 DSP 依赖
- `Middlewares/`：ARM DSP 库
- `SG/`：控制、采样、保护、PWM、调度和显示模块
- `OT/`：工程附加模块
- `.vscode/`：VSCode IntelliSense 与 Keil 编译任务
- `MDK-ARM/1.uvprojx`：Keil 主工程
- `MDK-ARM/1.uvoptx`：Keil 工程选项
- `MDK-ARM/startup_stm32g474xx.s`：启动文件
- `MDK-ARM/stm32g474xx_flash.sct`：链接脚本
- `1.ioc`：STM32CubeMX 配置

`MDK-ARM/1/`、编译日志、列表文件和个人界面配置均为可重新生成内容，不纳入版本控制。

## 开发环境

- MCU：STM32G474RBT6
- IDE：Keil MDK-ARM 5
- 编译器：Arm Compiler 6（已使用 6.24 验证）
- 编辑器：VSCode
- 配置工具：STM32CubeMX

## 编译方法

### Keil

打开 `MDK-ARM/1.uvprojx`，执行 **Rebuild all target files**。

### VSCode

1. 用 VSCode 打开仓库根目录。
2. 执行 `Terminal -> Run Build Task`，或按 `Ctrl+Shift+B`。
3. 默认任务调用 `.vscode/build-keil.ps1`。

如果 Keil 未安装在常用路径，可设置环境变量 `UV4_PATH` 或 `KEIL_PATH`，指向 `UV4.exe`。

## 团队协作

首次获取工程：

```powershell
git clone <private-repository-url>
cd STM32G474-ThreePhase-Inverter-Control
```

开始修改前：

```powershell
git pull
git switch -c feature/<功能名称>
```

完成修改后：

```powershell
git add <修改的文件>
git commit -m "说明本次修改"
git push -u origin feature/<功能名称>
```

建议通过 Pull Request 合并到 `main`，避免多人同时直接修改主分支。

## 上电安全

三相逆变器涉及高压、大电流和上下桥臂直通风险。修改 PWM、死区、保护阈值或驱动极性后，应先在低压限流条件下验证六路驱动波形、死区与故障关断逻辑，再连接功率母线和负载。

