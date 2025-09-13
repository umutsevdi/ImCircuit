#include <doctest.h>
#include "core.h"

using namespace ic;
TEST_CASE("add-remove-re-add")
{
    Scene s;
    Node i1 = s.add_node<Input>();
    Node o  = s.add_node<Output>();
    relid r = s.connect(o, 0, i1);
    REQUIRE(r);
    s.get_node<Input>(i1)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    REQUIRE_EQ(s.remove_node(i1), Error::OK);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);
    i1 = s.add_node<Input>();
    REQUIRE(s.connect(o, 0, i1));
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);
    s.get_node<Input>(i1)->set(true);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
}

TEST_CASE("Read Gate Test")
{
    Scene s;
    Node i1    = s.add_node<Input>();
    Node i2    = s.add_node<Input>();
    Node o     = s.add_node<Output>();
    Node g_and = s.add_node<Gate>(Gate::Type::AND);
    REQUIRE(s.connect(o, 0, g_and));

    relid r = s.connect(g_and, 0, i1);
    REQUIRE(r);
    REQUIRE(s.connect(g_and, 1, i2));
    s.get_node<Input>(i1)->set(true);
    s.get_node<Input>(i2)->set(false);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);

    REQUIRE_EQ(s.remove_node(g_and), Error::OK);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);

    Node g_or = s.add_node<Gate>(Gate::Type::OR);
    REQUIRE(s.connect(o, 0, g_or));
    REQUIRE(s.connect(g_or, 0, i1));
    REQUIRE(s.connect(g_or, 1, i2));

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
}

TEST_CASE("Gate State Change After Removal")
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
    REQUIRE_EQ(s.remove_node(g_or), Error::OK);
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);

    g_or = s.add_node<Gate>(Gate::Type::OR);
    REQUIRE(s.connect(g_or, 0, i1));
    REQUIRE(s.connect(g_or, 1, i2));
    REQUIRE(s.connect(o, 0, g_or));
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);

    s.get_node<Input>(i1)->set(false);
    s.get_node<Input>(i1)->set(false);

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::FALSE);
}
