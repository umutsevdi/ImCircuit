#include <doctest.h>
#include "core.h"

using namespace lcs;
TEST_CASE("connect-disconnect-reconnect")
{
    Scene s;
    Node i = s.add_node<Input>();
    Node o = s.add_node<Output>();
    relid r = s.connect(o, 0, i);
    REQUIRE(r);
    s.get_node<Input>(i)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    s.disconnect(r);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);
    REQUIRE(s.connect(o, 0, i) != 0);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
}

TEST_CASE("disconnect-early")
{
    Scene s;
    Node i1    = s.add_node<Input>();
    Node i2    = s.add_node<Input>();
    Node o     = s.add_node<Output>();
    Node g_and = s.add_node<Gate>(Gate::Type::AND);

    relid r = s.connect(g_and, 0, i1);
    REQUIRE(r);
    REQUIRE(s.connect(g_and, 1, i2));
    REQUIRE(s.connect(o, 0, g_and));
    s.get_node<Input>(i1)->set(true);
    s.get_node<Input>(i2)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    s.disconnect(r);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);
    REQUIRE_EQ(s.get_node<Gate>(g_and)->get(), State::DISABLED);
}

TEST_CASE("change-state-after-disconnect")
{
    Scene s;
    Node g_or  = s.add_node<Gate>(Gate::Type::OR);
    Node g_and = s.add_node<Gate>(Gate::Type::AND);
    Node i1    = s.add_node<Input>();
    Node i2    = s.add_node<Input>();
    Node i3    = s.add_node<Input>();
    Node o     = s.add_node<Output>();

    REQUIRE(s.connect(g_or, 0, i1));
    REQUIRE(s.connect(g_or, 1, i2));
    REQUIRE(s.connect(g_and, 0, i3));
    REQUIRE(s.connect(g_and, 1, i1));
    REQUIRE(s.connect(o, 0, g_or));

    s.get_node<Input>(i1)->set(true);
    s.get_node<Input>(i2)->set(false);
    s.get_node<Input>(i3)->set(true);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);

    s.disconnect(s.connect(g_or, 0, i1));
    s.get_node<Input>(i1)->set(false);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);
}

TEST_CASE("invalid-connection")
{
    Scene s;
    Node o1 = s.add_node<Output>();
    Node o2 = s.add_node<Output>();
    Node i1 = s.add_node<Input>();

    REQUIRE_FALSE(s.connect(o1, 0, o2));
    REQUIRE_FALSE(s.connect(i1, 0, o1));
    REQUIRE(s.connect(o1, 0, i1));
    REQUIRE_FALSE(s.connect(o1, 0, i1));
}

TEST_CASE("reconnect-after-multiple-disconnects")
{
    Scene s;
    Node g_and = s.add_node<Gate>(Gate::Type::AND);
    Node i1    = s.add_node<Input>();
    Node i2    = s.add_node<Input>();
    Node o     = s.add_node<Output>();

    relid r1 = s.connect(g_and, 0, i1);
    relid r2 = s.connect(g_and, 1, i2);
    REQUIRE(r1);
    REQUIRE(r2);
    relid r3 = s.connect(o, 0, g_and);
    REQUIRE(r3);

    s.get_node<Input>(i1)->set(true);
    s.get_node<Input>(i2)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);

    s.disconnect(r1);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);
    s.disconnect(r2);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);

    REQUIRE(s.connect(g_and, 0, i1));
    REQUIRE(s.connect(g_and, 1, i2));
    REQUIRE_EQ(s.get_node<Gate>(g_and)->get(), State::TRUE);

    s.disconnect(r3);
    REQUIRE_EQ(s.get_node<Gate>(g_and)->get(), State::TRUE);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);

    REQUIRE(s.connect(o, 0, g_and));
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
}
