# 编码规范

## 通用原则

1. **代码注释用英文（ASCII）**，工程文档用中文；禁止在源码中出现非 ASCII 字符（避免 MSVC 无 BOM 乱码）。
2. 行尾统一 LF（`.gitattributes` 已强制）；UTF-8 编码。
3. 禁止 `using namespace std;` 于头文件；命名空间内使用 `using` 需限定。
4. 所有公共接口必须有头文件注释（职责、线程安全说明、调用约束）。

## C++（software/，C++17）

- 命名：类型 `PascalCase`（`BusFrame`）；函数/变量 `snake_case`（`decode_frame`）；常量 `kPascalCase` 或 `SCREAMING_SNAKE`；成员 `snake_case_` 尾下划线；宏 `BT_XXX`。
- 头文件：`#pragma once`；每个头文件自成一体（自包含 include guard 依赖）。
- 智能指针优先：`std::unique_ptr` > `std::shared_ptr`；裸指针仅限非拥有观察者。
- 避免异常跨模块边界：HAL/引擎内部允许异常，接口层必须捕获并转为 `Result`/错误码。
- 线程：共享状态必须加锁或文档化"单线程拥有"；跨线程传递用消息队列，禁止隐式共享。
- 单元测试：每个公共模块必须配 `tests/unit/` 用例（使用 `minitest.h` 或 gtest，二者皆可）。

## C（firmware/，C11）

- 遵循 MISRA-C:2012 子集（rule 10.x/12.x 重点关注），强制静态分析（`-fanalyzer` / cppcheck）。
- 中断服务程序（ISR）：禁止阻塞、禁止动态内存、禁止长循环；ISR 只置标志/入队。
- 寄存器访问：统一 `LL` 层或寄存器位域封装，禁止裸 magic number。
- 模块接口：`模块名_动词_名词()`（如 `fdcan_init()`、`usb_ep_send()`）。

## Python（software/python/，3.11）

- PEP 8；类型注解必须（`def handle(msg: BusFrame) -> None`）。
- 包命名 `buspytest`；公开 API 集中在 `api.py`，DSL 在 `dsl.py`，CAPL 兼容在 `capl.py`。
- 禁止在导入时执行副作用；可测试性优先。

## Verilog（hardware/fpga/）

- 命名：模块 `snake_case`；信号 `snake_case`；常量 `SCREAMING_SNAKE`；时钟 `clk`，复位 `rst_n`（低有效）。
- 同步时序逻辑统一 `always @(posedge clk)`；组合逻辑用 `always @(*)` 或 `assign`，禁止 latch。
- 跨时钟域（CDC）必须用同步器/异步 FIFO，并标注 `// CDC:` 注释。
- 每个模块必须配套 testbench；仿真通过后方可上板。

## 构建

- 构建系统：CMake ≥ 3.24 + Ninja（本地 / CI 统一）。
- 编译警告：`-Wall -Wextra -Wpedantic`（CI 开启 `-Werror`）；zig 本地验证使用 `-Wno-everything` 仅做语法/链接验证。
- 禁止提交构建产物与 IDE 缓存（见 `.gitignore`）。
