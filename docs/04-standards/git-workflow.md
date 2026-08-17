# Git 工作流

## 分支模型（Trunk-Based + 短生命周期特性分支）

```
main  ──────●───────────●───────────●──────  (始终可发布)
             \          / \         /
feature/fpga-timestamp ──   fix/usb-overtun
```

- `main`：唯一主干，**始终可构建、可发布**；禁止直接推送，合并须过 PR + CI 绿灯。
- 特性分支：`feature/<域>-<简述>`（如 `feature/fpga-timestamp`、`feature/rbs-scheduler`）。
- 修复分支：`fix/<简述>`；里程碑分支：`release/v0.1`（可选，发布时冻结）。

## 提交规范（Conventional Commits）

```
<type>(<scope>): <subject>

<body>   # 可选的详细说明，说明"为什么"而非"做了什么"
```

- type：`feat` / `fix` / `docs` / `refactor` / `test` / `build` / `ci` / `style` / `perf` / `chore`
- scope：`fpga` / `fw` / `sw-core` / `sw-protocol` / `sw-engine` / `sw-ui` / `hal` / `test-eng` / `ci` / `docs`
- 示例：`feat(fw): add FDCAN tx scheduling with TT mode`

## PR 流程

1. 分支 → 提交（粒度小，单提交单主题）→ 推送。
2. 发起 PR：模板含「需求链接 / 变更摘要 / 测试结果 / 影响面」。
3. CI 全绿（build + unit tests + lint）后方可合并；至少 1 名 reviewer 批准。
4. 合并策略：Squash Merge（主干保持线性历史）。

## 版本号（SemVer）

`MAJOR.MINOR.PATCH`（如 `v0.1.0`）。里程碑 M1/M2/M3 对应 v0.1/v0.2/v0.3，正式发布 v3.0。

## 禁止事项

- 禁止 `--force` 推送共享分支；禁止跳过 CI（`--no-verify`）。
- 禁止在提交中混入无关文件（IDE 缓存、构建产物）。
- 禁止直接修改他人未合入分支的核心接口（先沟通或开 ADR）。
