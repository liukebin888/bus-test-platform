// test_framework.h - Minimal zero-dependency unit test harness
//
// Each test TU calls BT_TEST(name) { ... } with BT_CHECK/BT_CHECK_EQ
// assertions. Registration is static-init based; test_main.cpp provides
// the runner. Deliberately dependency-free (no gtest) so the skeleton
// builds with any C++17 toolchain, including the sandbox bootstrap.
#pragma once

#include <cstdio>
#include <exception>
#include <functional>
#include <utility>
#include <vector>

namespace bt {
namespace test {

struct TestCase {
    const char* name;
    std::function<bool()> fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> r;
    return r;
}

struct Registrar {
    Registrar(const char* name, std::function<bool()> fn) {
        registry().push_back(TestCase{name, std::move(fn)});
    }
};

inline int run_all() {
    int passed = 0;
    int failed = 0;
    for (const auto& t : registry()) {
        bool ok = false;
        try {
            ok = t.fn();
        } catch (const std::exception& e) {
            std::fprintf(stderr, "    [EXC] %s: %s\n", t.name, e.what());
        } catch (...) {
            std::fprintf(stderr, "    [EXC] %s: unknown exception\n", t.name);
        }
        if (ok) {
            ++passed;
            std::printf("PASS %s\n", t.name);
        } else {
            ++failed;
            std::printf("FAIL %s\n", t.name);
        }
    }
    std::printf("----\n%d passed, %d failed, %zu total\n", passed, failed,
                registry().size());
    return failed == 0 ? 0 : 1;
}

}  // namespace test
}  // namespace bt

#define BT_TEST(name)                                                     \
    static bool name();                                                   \
    static ::bt::test::Registrar bt_test_reg_##name(#name, name);         \
    static bool name()

#define BT_CHECK(cond)                                                    \
    do {                                                                  \
        if (!(cond)) {                                                    \
            std::fprintf(stderr, "    CHECK failed %s:%d: %s\n",          \
                         __FILE__, __LINE__, #cond);                      \
            return false;                                                 \
        }                                                                 \
    } while (0)

#define BT_CHECK_EQ(a, b)                                                 \
    do {                                                                  \
        const auto bt_va = (a);                                           \
        const auto bt_vb = (b);                                           \
        if (!(bt_va == bt_vb)) {                                          \
            std::fprintf(stderr, "    EQ failed %s:%d: %s == %s\n",       \
                         __FILE__, __LINE__, #a, #b);                     \
            return false;                                                 \
        }                                                                 \
    } while (0)
