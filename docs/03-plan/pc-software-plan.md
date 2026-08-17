# PC 端软件细化计划（Phase A · W09-14）

> 本文将 development-plan.md 中 "W09-14 PC 端重构：对象模型 + HAL + 零拷贝管线"
> 分解为可执行、可验收的工作包（WP-PC1 ~ WP-PC6）。
> 硬件域已另行安排，PC 端全程以 Null/Sim 设备开展，板卡到位后做真机对接（W11-14 插入）。

## 工作包分解

| 工作包 | 内容 | 交付物 / 验收标准 | 状态 |
|---|---|---|---|
| WP-PC1 仿真数据源 | SimUsbDevice：可配置速率/通道掩码/帧型式的 HAL 级流量生成器，wall-clock 到期帧模型 | `hal/usb/usb_device_sim.*`；单测覆盖配置收敛/闭合读取/限界生成/重开复位 | **done** 2026-08-17 |
| WP-PC2 采集服务 | CaptureService：生产者线程（HAL → Pipeline），batch=256，可启停/可重启，offered/pushed/poll_cycles 统计 | `data/capture_service.*`；单测覆盖未开设备拒绝/搬运正确性/计数守恒/重启确定性 | **done** 2026-08-17 |
| WP-PC3 基准工具 | busmon bench：①SPSC 管线吞吐 ②push→pop 延迟 ③溢出记账 ④codec 往返 ⑤端到端（sim→采集线程→消费者）；M1 主机侧门禁判定（≥50k msg/s、<1ms、丢帧记账） | `src/bench/*` + `busmon --bench [--duration=S] [--rate=N]`，exit code 0/2 | **done** 2026-08-17 |
| WP-PC4 对象模型深化 | Channel 统计（错误帧/总线负载）、Signal 解码链（DBC 物理值 → Workspace 绑定）、过滤器模型 | 单测 + demo 输出物理值解码 | 待启动 |
| WP-PC5 trace 落盘 | busmon capture：帧流写 `.btrace` 二进制 + 回放；M1 报告素材导出（CSV） | 单测：写→读 round-trip 无损 | 待启动 |
| WP-PC6 单测与 CI 扩充 | 用例从 21 → 40+；CI 增加 bench 冒烟 job（Ubuntu runner） | CI 全绿；覆盖率不回退 | 持续 |

## 实施顺序与依赖

```
WP-PC1 ─→ WP-PC2 ─→ WP-PC3     （数据流通路：生成 → 搬运 → 测量）
              └────→ WP-PC5     （通路就绪后落盘才有意义）
WP-PC4 独立并行（纯模型层，无 HAL 依赖）
WP-PC6 随每个工作包同步扩充
```

## 真机对接预留（板卡到位后）

- `UsbDevice` 接口不变，新增 `LibUsbDevice` 实现（libusb-1.0 / WinUSB 后端，Phase A 末期）
- bench 的 stage 4（端到端）直接切换设备实现即可复用全部测量逻辑
- 时间戳链路：FPGA ns100 硬件戳 → 帧字段 → 消费者延迟统计，已按 100ns tick 对齐

## 工程规则

1. 每个工作包完成即本地 `ci-build.sh --only software` 全绿 + git commit。
2. 新增公共接口先补单测再接入 busmon。
3. bench 门禁阈值与 milestones.md M1 验收标准保持一一对应，改动需同步更新两处。
