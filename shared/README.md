# shared/ — 跨子项目共享定义

单一事实来源（Single Source of Truth）目录。**固件（C11）与 PC 软件（C++17）共同引用**，
任何修改必须触发全仓 CI 构建，防止跨域接口漂移。

| 文件 | 内容 |
|---|---|
| `include/bus/bus_types.h` | 总线类型、方向、帧状态、`bt_bus_frame_t` 统一帧结构（方案 6.6） |
| `include/bus/usb_protocol.h` | USB 链路协议：端点规划、命令集、批量 16 帧/包帧格式（方案 4.4） |

## 引用方式

- 固件：`target_include_directories(... ../shared/include)`
- 软件：`target_include_directories(... ../shared/include)`（经 `bt_shared` INTERFACE 库）

## 变更纪律

1. 结构体布局（packed）变更 = 固件/软件同步发布，禁止单侧修改。
2. 新增命令/事件：先改本目录头文件，再同步实现两侧编解码。
3. 协议版本号 `BT_USB_PROTOCOL_VERSION` 递增并在变更说明中登记。
