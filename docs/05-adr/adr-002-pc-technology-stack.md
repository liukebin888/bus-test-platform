# ADR-002：PC 端技术栈（Qt6 + C++17 + Python 3.11）

- 状态：**已接受**（2026-08-17）
- 提出人：软件架构师

## 背景

PC 应用需满足：60fps 表格/图形渲染、确定性仿真引擎、脚本自动化（Python DSL + CAPL 兼容）、
多格式报告、跨平台（Windows/macOS/Linux）。

## 决策

| 层次 | 选型 | 理由 |
|---|---|---|
| 桌面框架 | Qt 6（C++17）+ QML/Widgets | 跨平台、高性能表格/图形、与流程画布同栈 |
| 脚本引擎 | Python 3.11（pybind11 嵌入） | DSL 语法简洁、numpy/matplotlib 生态 |
| 数据库 | SQLite（单机）→ PostgreSQL（团队） | 零运维 → 协作升级，数据模型一致 |
| 报告 | Jinja2 + Chart.js + openpyxl + python-docx + 无头 Chromium | HTML/PDF/Excel/Word 多格式版式一致 |
| 日志 | BLF / ASC / MF4 / CSV / JSON | 与 CANoe 格式兼容，MMAP 异步落盘 |
| 构建 | CMake + vcpkg + CI | 可复现构建 + 自动化回归 |

工程骨架默认**不依赖 Qt 即可构建**（`core/protocol/engine/data/hal` 为纯 C++17，
UI 层通过 `BT_ENABLE_UI=ON` 显式开启），保证核心逻辑可独立测试与快速迭代。

## 备选方案

1. **C# / WPF + .NET**：Windows 友好但放弃 Linux/macOS 与 CANoe 生态对齐；拒绝。
2. **Electron/TypeScript**：UI 迭代快但实时数据处理与 60fps 渲染受限；拒绝。
3. **纯 C++ + Dear ImGui**：性能佳但工程管理类界面（表格/流程画布）开发成本高；拒绝。

## 后果

- 团队需同时具备 C++ 与 Python 能力（已在团队配置中覆盖：C++ 桌面 3-4 人 + 脚本工具链 1-2 人）。
- Python 嵌入采用 pybind11，GIL 与主线程交互按方案 6.7 线程模型隔离。
- 本地无 Qt 环境时，开发核心逻辑不受阻；UI 联调依赖 Qt 安装或 CI 容器。
