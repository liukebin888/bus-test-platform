# reports/ - 测试报告

CI 与本地执行在此产出测试报告（JUnit XML / 文本摘要 / 覆盖度）。

| 文件 | 生成时机 | 说明 |
|------|---------|------|
| `sw-unit-latest.txt` | 每次 CI host 测试 | PC 端单测 PASS/FAIL 摘要 |
| `fw-host-latest.txt` | 每次 CI host 测试 | 固件 host 协议单测摘要 |

Phase C 前补齐：覆盖率报告（gcov/lcov）、系统测试报告模板。
