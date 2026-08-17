# tools/ - 本地工具链与辅助脚本

| 文件 | 说明 |
|------|------|
| `local_build.sh` | 一键构建（software/firmware host 测试），无系统编译器时自动用 zig 自举 |

## 沙箱 zig 自举（WorkBuddy 无系统工具链环境）

本仓库的 PC 端骨架用纯 C++17 编写，理论上任何工具链可编译。在没有任何
系统编译器的沙箱里，可用 pip 安装的 zig（内含 clang 21）验证：

```bash
# 1) 安装工具（一次性）
pip install cmake ninja ziglang

# 2) 编译器包装：zig 的 libc++ 头文件在 Windows 产生海量告警，需屏蔽
#    .cmd 包装（zig-c++.cmd）:  zig c++ -Wno-everything -Wno-error %*

# 3) 归档器包装：CMake 探测到 clang 编译器后强制探测 llvm-ar，且会忽略
#    -DCMAKE_AR 覆盖。破解点：把转发 `zig ar` 的 exe 命名为 llvm-ar.exe
#    放在编译器同目录（探测即命中）。arwrap.c 即该包装源码，用
#    `zig cc arwrap.c -o arwrap.exe` 编译后复制为 llvm-ar.exe。
```

> 踩坑记录：CMake 4.4 对 .cmd 编译器包装器支持良好，但归档器不接受
> .cmd/.bat；`CMAKE_CXX_ARCHIVE_CREATE` / `CMAKE_CXX_COMPILER_AR` 在
> Ninja 生成器下均被 clang 探测逻辑覆盖，唯一可靠方式是让探测找到
> `llvm-ar.exe`（见上）。真实 CI（Ubuntu GCC/Clang）无此问题。
