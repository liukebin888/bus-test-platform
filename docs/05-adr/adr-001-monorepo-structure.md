# ADR-001：Monorepo 单仓库工程结构

- 状态：**已接受**（2026-08-17）
- 提出人：软件架构师 / PM

## 背景

平台横跨硬件（原理图/PCB/BOM）、FPGA RTL、STM32 固件、Qt6 PC 软件、测试工程五个专业域，
10-15 人团队并行开发。需要选择仓库组织方式。

## 决策

采用 **Monorepo（单仓库）**，目录即专业域边界：

```
docs/  shared/  hardware/  firmware/  software/  test-engineering/  ci/  tools/
```

- 各子工程独立 CMake 项目（`software/`、`firmware/` 各自有 `CMakeLists.txt` 与 Presets）。
- 跨域共享定义收敛到 `shared/include/bus/`（总线类型、USB 协议帧格式），固件与 PC 端共同引用，保证单一事实来源。
- 硬件/FPGA 的产出（原理图、RTL）与固件/软件在**同一提交**内演进，跨域接口变更可原子落地。

## 备选方案

1. **多仓库**（每个域一个 repo）：跨域接口变更需跨 repo 发 PR，协议帧格式易漂移；拒绝。
2. **单仓库但共享头文件双份拷贝**：违反单一事实来源，编译期即发现漂移；拒绝。

## 后果

- 优点：原子变更、跨域重构成本低、CI 可在一次流水线内构建全部子工程。
- 代价：仓库体积随原理图/PCB 二进制增长；需用 `.gitignore` + LFS（原理图/PCB 后期考虑）。
- 约定：**任何跨域接口（USB 帧格式、BusFrame）变更必须同步修改 `shared/` 并触发全仓 CI**。
