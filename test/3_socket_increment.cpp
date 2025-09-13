#include <doctest.h>
#include "core.h"

using namespace ic;
TEST_CASE("increment-socket-size")
{
    Scene s;

    Node i1    = s.add_node<Input>();
    Node i2    = s.add_node<Input>();
    Node g_and = s.add_node<Gate>(Gate::Type::AND);
    Node o     = s.add_node<Output>();

    s.get_node<Input>(i1)->set(true);
    s.get_node<Input>(i2)->set(true);

    REQUIRE(s.connect(o, 0, g_and));
    REQUIRE(s.connect(g_and, 0, i1));
    REQUIRE(s.connect(g_and, 1, i2));

    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
    s.get_node<Gate>(g_and)->increment();
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::DISABLED);
    s.get_node<Gate>(g_and)->decrement();
    REQUIRE_EQ(s.get_node<Output>(o)->get(), State::TRUE);
}
