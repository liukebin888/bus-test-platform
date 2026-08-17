# 接入真实 GitHub Actions Runner — 前置清单

本文档回答一个问题：**从当前本地工作区到"CI 在真实 Ubuntu runner 上跑绿"**，
需要哪些步骤、验证什么、以及受限环境的替代验证方式。

---

## 1. CI 流水线验证范围（当前 ci.yml 定义）

| Job | Runner | 验证内容 |
|---|---|---|
| `sw-host` | ubuntu-latest × 2 | 矩阵 `g++` / `clang++`：Release 构建 25 目标 + `bt_unit_tests`（21 项）+ `busmon --info` 冒烟 |
| `fw-host` | ubuntu-latest | C11 固件协议编解码 host 测试（`fw_protocol_test`，字节级用例） |
| `rtl-syntax` | ubuntu-latest | iverilog `-g2012` 全量编译 FPGA 源文件，逐顶层检查 7 个模块（top + 6 IP） |
| `docs` | ubuntu-latest | 占位（Phase B 启用） |

构建语义唯一入口：`ci/scripts/ci-build.sh`（workflow 与本地共用，避免双份漂移）。

## 2. 前置条件

1. **GitHub 仓库**：新建空仓库（如 `bus-test-platform`，建议私有）。
2. **连接方式二选一**：
   - **推荐**：在 WorkBuddy「连接器」中连接 **GitHub** Connector（本会话初始未连接），
     授权后可代建仓库/推送；
   - 或手动：本机 git 配置 `user.name` / `user.email` 后直接 `git push`。

## 3. 接入步骤（本地已可执行部分）

```bash
cd bus-test-platform

# 1) 初始化仓库并提交（.gitignore 已排除 build/ 与产物）
git init -b main
git add .
git commit -m "chore: 汽车总线测试平台 Monorepo 工程骨架（Phase A 起点）"

# 2) 关联远程（以 GitHub 仓库为例）
git remote add origin git@github.com:<org>/bus-test-platform.git
# 或 HTTPS：git remote add origin https://github.com/<org>/bus-test-platform.git

# 3) 推送（触发 push 流水线）
git push -u origin main
```

推送后到仓库 **Actions** 页确认：3 个 job 全绿为验收通过。

## 4. 本地复现 CI 语义（无 Ubuntu 环境时）

### 4.1 沙箱/无系统编译器环境（zig 自举）

```bash
# 编译器包装脚本位于 ../.toolcheck/（zig-c++.cmd / zig-cc.cmd）
CXX="D:/.../.toolcheck/zig-c++.cmd" \
CC="D:/.../.toolcheck/zig-cc.cmd" \
bash ci/scripts/ci-build.sh          # software + firmware 全链
```

### 4.2 有 Docker 的机器（可选，act 本地跑 GitHub Actions）

```bash
# 需要本机装有 Docker；ci.yml 的 ubuntu-latest 镜像在 act 下可用
act -j sw-host
act -j fw-host
```

> 局限：`rtl-syntax` 依赖 `apt-get install iverilog`，act 需
> `--container-architecture linux/amd64` 且镜像支持 apt；最可靠仍是真实 runner。

## 5. 已知边界与后续

- **rtl-syntax 已修复**：`top.v` 实例化 6 个子模块，单文件编译会报
  `Unknown module type`；现改为全量 `*.v` 编译 + 逐顶层 `-s` 检查。
- **sw-host 已修复**：矩阵编译器为 `g++`/`clang++`（原 `gcc`/`clang` 名不规范）；
  统一调用 `ci-build.sh --only software`，`CXX` 环境变量注入。
- **Phase B**：`docs` job 启用文档链接/需求追溯（TC↔REQ）检查。

## 6. 验收清单（push 后）

- [ ] Actions 触发 `push: main`
- [ ] `sw-host (g++)`：25/25 编译、21/21 单测、`busmon --info` 输出 OK
- [ ] `sw-host (clang++)`：同上（clang 更严，`-Wall -Wextra` 无新增告警）
- [ ] `fw-host`：`fw_protocol_test` 1/1 Passed
- [ ] `rtl-syntax`：7 个顶层模块语法检查全过
- [ ] 总耗时预期 < 5 min（不含依赖安装）
