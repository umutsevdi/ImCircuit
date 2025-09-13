#include <doctest.h>
#include "common.h"
#include "core.h"

using namespace ic;

TEST_CASE("set-input")
{
    Scene s;
    Node v = s.add_node<Input>();
    Node o = s.add_node<Output>();
    REQUIRE(s.connect(o, 0, v));
    ic_assert(s.get_node<Output>(o) != nullptr);
    ic_assert(s.get_node<Input>(v) != nullptr);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);
    s.get_node<Input>(v)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
}

TEST_CASE("disconnect")
{
    Scene s;
    Node v = s.add_node<Input>();
    Node o = s.add_node<Output>();
    relid r = s.connect(o, 0, v);
    REQUIRE(r != 0);
    s.get_node<Input>(v)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    s.disconnect(r);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);
}

TEST_CASE("and(i1, i1)->o")
{
    Scene s;
    Node v     = s.add_node<Input>();
    Node o     = s.add_node<Output>();
    Node g_and = s.add_node<Gate>(Gate::Type::AND);

    REQUIRE(s.connect(g_and, 0, v));
    REQUIRE(s.connect(g_and, 1, v));
    REQUIRE(s.connect(o, 0, g_and));

    s.get_node<Input>(v)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    s.get_node<Input>(v)->set(false);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);
}

TEST_CASE("or(and(i1, i2), i2)->o")
{
    Scene s;
    Node g_and = s.add_node<Gate>(Gate::Type::AND);
    Node g_or  = s.add_node<Gate>(Gate::Type::OR);
    Node i1    = s.add_node<Input>();
    Node i2    = s.add_node<Input>();
    Node o     = s.add_node<Output>();

    REQUIRE(s.connect(g_and, 0, i1));
    REQUIRE(s.connect(g_and, 1, i2));
    REQUIRE(s.connect(g_or, 0, g_and));
    REQUIRE(s.connect(g_or, 1, i1));
    REQUIRE(s.connect(o, 0, g_or));

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);

    s.get_node<Input>(i1)->set(true);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    REQUIRE_EQ(s.get_node<Gate>(g_and)->get(), State::FALSE);
    REQUIRE_EQ(s.get_node<Gate>(g_or)->get(), State::TRUE);
}

TEST_CASE("and(i1, i2), or(i1, i2)->o, nand(i2, i3), nor(i3, i1)")
{
    Scene s;
    Node g_and  = s.add_node<Gate>(Gate::Type::AND);
    Node g_or   = s.add_node<Gate>(Gate::Type::OR);
    Node g_nand = s.add_node<Gate>(Gate::Type::NAND);
    Node g_nor  = s.add_node<Gate>(Gate::Type::NOR);
    Node i1     = s.add_node<Input>();
    Node i2     = s.add_node<Input>();
    Node i3     = s.add_node<Input>();
    Node o      = s.add_node<Output>();

    REQUIRE(s.connect(g_and, 0, i1));
    REQUIRE(s.connect(g_and, 1, i2));
    REQUIRE(s.connect(g_or, 0, i1));
    REQUIRE(s.connect(g_or, 1, i2));
    REQUIRE(s.connect(g_nand, 0, i2));
    REQUIRE(s.connect(g_nand, 1, i3));
    REQUIRE(s.connect(g_nor, 0, i3));
    REQUIRE(s.connect(g_nor, 1, i1));
    REQUIRE(s.connect(o, 0, g_or));

    s.get_node<Input>(i1)->set(true);
    s.get_node<Input>(i2)->set(true);
    s.get_node<Input>(i3)->set(false);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    REQUIRE_EQ(s.get_node<Gate>(g_and)->get(), State::TRUE);
    REQUIRE_EQ(s.get_node<Gate>(g_or)->get(), State::TRUE);
    REQUIRE_EQ(s.get_node<Gate>(g_nand)->get(), State::TRUE);
    REQUIRE_EQ(s.get_node<Gate>(g_nor)->get(), State::FALSE);
}

TEST_CASE("1-bit-adder")
{
    Scene s;
    Node v1   = s.add_node<Input>();
    Node v2   = s.add_node<Input>();
    Node gate = s.add_node<Gate>(Gate::Type::OR);
    Node sum  = s.add_node<Output>();

    REQUIRE(s.connect(gate, 0, v1));
    REQUIRE(s.connect(gate, 1, v2));
    REQUIRE(s.connect(sum, 0, gate));

    SUBCASE("1 + 0 = 1")
    {
        s.get_node<Input>(v1)->set(true);
        s.get_node<Input>(v2)->set(false);
        REQUIRE_EQ(s.get_node<Output>(sum)->get(), State::TRUE);
    }
    SUBCASE("0 + 1 = 1")
    {
        s.get_node<Input>(v1)->set(false);
        s.get_node<Input>(v2)->set(true);
        REQUIRE_EQ(s.get_node<Output>(sum)->get(), State::TRUE);
    }
    SUBCASE("0 + 0 = 0")
    {
        s.get_node<Input>(v1)->set(false);
        s.get_node<Input>(v2)->set(false);
        REQUIRE_EQ(s.get_node<Output>(sum)->get(), State::FALSE);
    }
}

TEST_CASE("full-adder")
{
    Scene s;
    Node a     = s.add_node<Input>();
    Node b     = s.add_node<Input>();
    Node c_in  = s.add_node<Input>();
    Node c_out = s.add_node<Output>();
    Node sum   = s.add_node<Output>();

    Node g_xor       = s.add_node<Gate>(Gate::Type::XOR);
    Node g_xor_sum   = s.add_node<Gate>(Gate::Type::XOR);
    Node g_and_carry = s.add_node<Gate>(Gate::Type::AND);
    Node g_and       = s.add_node<Gate>(Gate::Type::AND);
    Node g_or        = s.add_node<Gate>(Gate::Type::OR);

    REQUIRE(s.connect(g_xor, 0, a));
    REQUIRE(s.connect(g_xor, 1, b));

    REQUIRE(s.connect(g_xor_sum, 0, g_xor));
    REQUIRE(s.connect(g_xor_sum, 1, c_in));
    REQUIRE(s.connect(sum, 0, g_xor_sum));

    REQUIRE(s.connect(g_and, 0, c_in));
    REQUIRE(s.connect(g_and, 1, g_xor));

    REQUIRE(s.connect(g_and_carry, 0, a));
    REQUIRE(s.connect(g_and_carry, 1, b));

    REQUIRE(s.connect(g_or, 0, g_and));
    REQUIRE(s.connect(g_or, 1, g_and_carry));
    REQUIRE(s.connect(c_out, 0, g_or));

    SUBCASE("01 + 00 = 01")
    {
        s.get_node<Input>(a)->set(true);
        s.get_node<Input>(b)->set(false);
        s.get_node<Input>(c_in)->set(false);
        REQUIRE_EQ(s.get_node<Output>(sum)->get(), State::TRUE);
        REQUIRE_EQ(s.get_node<Output>(c_out)->get(), State::FALSE);
    }

    SUBCASE("01 + 01 = 10")
    {
        s.get_node<Input>(a)->set(true);
        s.get_node<Input>(b)->set(true);
        s.get_node<Input>(c_in)->set(false);
        REQUIRE_EQ(s.get_node<Output>(sum)->get(), State::FALSE);
        REQUIRE_EQ(s.get_node<Output>(c_out)->get(), State::TRUE);
    }

    SUBCASE("10 + 01 = 11")
    {
        s.get_node<Input>(a)->set(true);
        s.get_node<Input>(b)->set(true);
        s.get_node<Input>(c_in)->set(true);
        REQUIRE_EQ(s.get_node<Output>(sum)->get(), State::TRUE);
        REQUIRE_EQ(s.get_node<Output>(c_out)->get(), State::TRUE);
    }
}
