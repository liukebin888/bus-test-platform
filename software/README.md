# software/ — PC 软件（Qt6 + C++17 五层架构）

对应方案第 6 章。工程骨架**默认不依赖 Qt 即可构建**：core/protocol/data/engine/hal 为纯 C++17，
UI 层通过 `BT_ENABLE_UI=ON` 显式开启（需 Qt6 Widgets）。

```
software/
├── CMakeLists.txt / CMakePresets.json / vcpkg.json
├── src/
│   ├── core/       # L-0 对象模型：BusFrame 帮助函数、Channel、Signal、Node、Workspace
│   ├── protocol/   # L3 协议栈：CAN 解码、DBC 解析、USB 帧编解码
│   ├── data/       # L2 数据处理：零拷贝环形缓冲管线
│   ├── engine/     # L4 业务引擎：RBS 调度器、脚本宿主接口、Test Runner、UDS 诊断
│   ├── hal/        # L1 HAL：USB 设备抽象 + Null 设备（自检）
│   └── ui/         # L5 UI（Qt6 可选）
├── tests/unit/     # 单测（minitest.h，零外部依赖）
└── python/         # Python SDK：buspytest 包（DSL + CAPL 兼容层 + API）
```

## 构建与测试

```bash
cmake -S software -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure     # 单测
./build/busmon --selftest                      # 最小自检 CLI
```

## 五层依赖方向

`ui → engine → protocol/data → core ← hal`（core 为共享底座；上层依赖下层，禁止反向）。

## 数据流

`NullUsbDevice/真实设备 → FrameRingBuffer(零拷贝) → CanDecoder/DbcParser → RbsScheduler/UI(16ms 批)`
