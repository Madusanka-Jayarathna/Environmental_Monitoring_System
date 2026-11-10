#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cmath>

namespace HostUnity {
    inline int g_tests_run = 0;
    inline int g_tests_failed = 0;
    inline std::string g_current_test;

    inline void record_failure(const char* file, int line, const std::string& msg) {
        std::cerr << "  FAIL: " << file << ":" << line << " - " << msg << std::endl;
        g_tests_failed++;
    }
}

struct HostTestRunner {
    using TestFunc = void(*)();
    static inline std::vector<std::pair<std::string, TestFunc>>& getTests() {
        static std::vector<std::pair<std::string, TestFunc>> tests;
        return tests;
    }
    static void registerTest(const std::string& name, TestFunc func) {
        getTests().push_back({name, func});
    }
    static int runAll() {
        std::cout << "\n==========================================" << std::endl;
        std::cout << "  Running Host Unit Tests (Unity Compat)   " << std::endl;
        std::cout << "==========================================" << std::endl;
        for (const auto& test : getTests()) {
            HostUnity::g_current_test = test.first;
            HostUnity::g_tests_run++;
            int prev_failures = HostUnity::g_tests_failed;
            std::cout << "[ RUN      ] " << test.first << std::endl;
            test.second();
            if (HostUnity::g_tests_failed == prev_failures) {
                std::cout << "[       OK ] " << test.first << std::endl;
            } else {
                std::cout << "[  FAILED  ] " << test.first << std::endl;
            }
        }
        std::cout << "==========================================" << std::endl;
        std::cout << "Total: " << HostUnity::g_tests_run << " tests | "
                  << "Passed: " << (HostUnity::g_tests_run - HostUnity::g_tests_failed) << " | "
                  << "Failed: " << HostUnity::g_tests_failed << std::endl;
        std::cout << "==========================================\n" << std::endl;
        return HostUnity::g_tests_failed > 0 ? 1 : 0;
    }
};

#define HOST_CONCAT_INNER(a, b) a##b
#define HOST_CONCAT(a, b) HOST_CONCAT_INNER(a, b)

#define TEST_CASE(name_str, tag_str) \
    static void HOST_CONCAT(host_test_func_, __LINE__)(); \
    struct HOST_CONCAT(HostTestReg_, __LINE__) { \
        HOST_CONCAT(HostTestReg_, __LINE__)() { \
            HostTestRunner::registerTest(name_str, HOST_CONCAT(host_test_func_, __LINE__)); \
        } \
    }; \
    static HOST_CONCAT(HostTestReg_, __LINE__) HOST_CONCAT(g_host_test_reg_, __LINE__); \
    static void HOST_CONCAT(host_test_func_, __LINE__)()

#define UNITY_BEGIN() 0
#define UNITY_END() HostTestRunner::runAll()
#define unity_run_all_tests() HostTestRunner::runAll()

#define TEST_ASSERT_TRUE(cond) \
    if (!(cond)) { HostUnity::record_failure(__FILE__, __LINE__, "Expression is not true: " #cond); }

#define TEST_ASSERT_FALSE(cond) \
    if ((cond)) { HostUnity::record_failure(__FILE__, __LINE__, "Expression is not false: " #cond); }

#define TEST_ASSERT_EQUAL_UINT32(expected, actual) \
    if ((uint32_t)(expected) != (uint32_t)(actual)) { \
        HostUnity::record_failure(__FILE__, __LINE__, "Expected: " + std::to_string(expected) + " but got: " + std::to_string(actual)); \
    }

#define TEST_ASSERT_EQUAL_HEX32(expected, actual) \
    if ((uint32_t)(expected) != (uint32_t)(actual)) { \
        HostUnity::record_failure(__FILE__, __LINE__, "Hex mismatch: expected " + std::to_string(expected) + " got " + std::to_string(actual)); \
    }

#define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual) \
    if (std::fabs((float)(expected) - (float)(actual)) > (float)(delta)) { \
        HostUnity::record_failure(__FILE__, __LINE__, "Float mismatch: expected " + std::to_string(expected) + " got " + std::to_string(actual)); \
    }
