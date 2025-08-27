#pragma once

#include <cstdio>
#include "common.h"
#include "doctest.h"
#define _create_full_adder_io(s)                                               \
    using namespace lcs;                                                       \
    Node a     = s.add_node<Input>();                                          \
    Node b     = s.add_node<Input>();                                          \
    Node c_in  = s.add_node<Input>();                                          \
    Node c_out = s.add_node<Output>();                                         \
    Node sum   = s.add_node<Output>();

#define _create_full_adder(s)                                                  \
    Node g_xor       = s.add_node<Gate>(Gate::Type::XOR);                      \
    Node g_xor_sum   = s.add_node<Gate>(Gate::Type::XOR);                      \
    Node g_and_carry = s.add_node<Gate>(Gate::Type::AND);                      \
    Node g_and       = s.add_node<Gate>(Gate::Type::AND);                      \
    Node g_or        = s.add_node<Gate>(Gate::Type::OR);                       \
                                                                               \
    s.connect(g_xor, 0, a);                                                    \
    s.connect(g_xor, 1, b);                                                    \
                                                                               \
    s.connect(g_xor_sum, 0, g_xor);                                            \
    s.connect(g_xor_sum, 1, c_in);                                             \
    s.connect(sum, 0, g_xor_sum);                                              \
                                                                               \
    s.connect(g_and, 0, c_in);                                                 \
    s.connect(g_and, 1, g_xor);                                                \
                                                                               \
    s.connect(g_and_carry, 0, a);                                              \
    s.connect(g_and_carry, 1, b);                                              \
                                                                               \
    s.connect(g_or, 0, g_and);                                                 \
    s.connect(g_or, 1, g_and_carry);                                           \
    s.connect(c_out, 0, g_or);

#include "core.h"
inline bool scene_cmp(lcs::Scene& s1, lcs::Scene& s2)
{
    if (s1.name != s2.name || s1.description != s2.description
        || s1.version != s2.version || s1._inputs.size() != s2._inputs.size()
        || s1._outputs.size() != s2._outputs.size()
        || s1._gates.size() != s2._gates.size()
        || s1._components.size() != s2._components.size()
        || s1.component_context.has_value()
            != s2.component_context.has_value()) {
        return false;
    }
    return true;
}

namespace lcs {
struct LcsReporter : public doctest::IReporter {
    const doctest::ContextOptions& opt;
    const doctest::TestCaseData* tc = nullptr;
    std::string name;

    LcsReporter(const doctest::ContextOptions& in)
        : opt(in)
    {
    }

    void report_query(const doctest::QueryData& /*in*/) override { }

    void test_run_start() override { }

    void test_run_end(const doctest::TestRunStats& in) override
    {
        fs::set_log_target("RESULT.txt");
        fs::_log(Message(Message::INFO, APPNAME_BIN, -1, "Reporter",
            "\r\n"
            "┌──────────────╢ Test Results ╟──────────────┐\r\n"
            "│   TEST CASE         ┬     %5d/%-5d      │\r\n"
            "│   ASSERTIONS CASE   │     %5d/%-5d      │\r\n"
            "└─────────────────────┴──────────────────────┘\r\n",
            in.numTestCases - in.numTestCasesFailed, in.numTestCases,
            in.numAsserts - in.numAssertsFailed, in.numAsserts));
        if (in.numAssertsFailed > 0) {
            fs::_log(Message(Message::INFO, APPNAME_BIN, -1, "Reporter",
                "%d assertions failed in %d test cases", in.numAssertsFailed,
                in.numTestCasesFailed));
        }
    }

    void test_case_start(const doctest::TestCaseData& in) override
    {
        tc = &in;
        std::filesystem::path file { in.m_file.c_str() };
        name = file.filename().replace_extension(".log").string();

        lcs::fs::set_log_target(name.c_str());
        fs::_log(Message(Message::INFO, APPNAME_BIN, -1, "start_test",
            "TEST CASE \"%s\"\r\n", in.m_name));
    }

    void test_case_reenter(const doctest::TestCaseData& /*in*/) override { }

    void test_case_end(const doctest::CurrentTestCaseStats& stats) override
    {
        fs::_log(Message(Message::INFO, APPNAME_BIN, -1, "Reporter",
            "\r\n"
            "┌────────╢ Test Case End [%.4f s] ╟────────┐\r\n"
            "│   ASSERTIONS CASE   │     %5d/%-5d      │\r\n"
            "└────────────────╢ %-7s ╟─────────────────┘\r\n",
            stats.seconds,
            stats.numAssertsCurrentTest - stats.numAssertsFailedCurrentTest,
            stats.numAssertsCurrentTest,
            stats.testCaseSuccess ? "SUCCESS" : "FAILURE"));
    }

    void test_case_exception(const doctest::TestCaseException& in) override
    {
        fs::_log(Message(Message::ERROR, name.c_str(), tc->m_line,
            "exception_handler",
            "Test case \"%s\" failed with an exception %s.", tc->m_name,
            in.error_string.c_str()));
    }

    void subcase_start(const doctest::SubcaseSignature& /*in*/) override { }

    void subcase_end() override { }

    void log_assert(const doctest::AssertData& in) override
    {
        if (!in.m_failed && !opt.success) {
            return;
        }
        if (in.m_threw) {
            fs::_log(Message(Message::ERROR, name.c_str(), in.m_line,
                "test_runner", "Assertion \"%s\" failed with an exception: %s.",
                in.m_expr, in.m_exception_string.c_str()));
        } else {
            fs::_log(Message(Message::ERROR, name.c_str(), in.m_line,
                "test_runner", "Assertion \"%s\" failed.", in.m_expr));
        }
    }

    void log_message(const doctest::MessageData& msg) override
    {
        fs::_log(Message(Message::INFO, name.c_str(), msg.m_line, "test_runner",
            "%s", msg.m_string.c_str()));
    }

    void test_case_skipped(const doctest::TestCaseData& /*in*/) override { }
};
} // namespace lcs
REGISTER_REPORTER("lcs", 1, lcs::LcsReporter);
