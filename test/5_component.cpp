#include <doctest.h>
#include "core.h"

using namespace lcs;

TEST_CASE("Create a component context")
{
    Scene s { ComponentContext { &s, 3, 1 }, "component-context" };
    Node g_and = s.add_node<Gate>(Gate::Type::AND);
    s.get_node<Gate>(g_and)->increment();

    REQUIRE(s.component_context.has_value());
    REQUIRE(s.connect(g_and, 0, s.component_context->get_input(0)));
    REQUIRE(s.connect(g_and, 1, s.component_context->get_input(1)));
    REQUIRE(s.connect(g_and, 2, s.component_context->get_input(2)));
    REQUIRE(s.connect(s.component_context->get_output(0), 0, g_and));

    SUBCASE("Run 3x AND Gate")
    {
        REQUIRE_EQ(s.component_context->run(0b111), 1);
        REQUIRE_EQ(s.component_context->run(0b110), 0);
        REQUIRE_EQ(s.component_context->run(0b101), 0);
        REQUIRE_EQ(s.component_context->run(0b100), 0);
        REQUIRE_EQ(s.component_context->run(0b011), 0);
        REQUIRE_EQ(s.component_context->run(0b010), 0);
        REQUIRE_EQ(s.component_context->run(0b001), 0);
        REQUIRE_EQ(s.component_context->run(0b00), 0);
    }
}

TEST_CASE("Create a 2x1 MUX component")
{
    // IN1 = 1, IN2 = 2, IN3 = SEL
    Scene s { ComponentContext { &s, 3, 1 }, "2x1-mux" };
    Node g_and   = s.add_node<Gate>(Gate::Type::AND);
    Node g_and_2 = s.add_node<Gate>(Gate::Type::AND);
    Node g_not   = s.add_node<Gate>(Gate::Type::NOT);
    Node g_out   = s.add_node<Gate>(Gate::Type::OR);

    s.connect(g_and, 0, s.component_context->get_input(0));
    s.connect(g_and_2, 0, s.component_context->get_input(1));
    s.connect(g_and, 1, s.component_context->get_input(2));
    s.connect(g_not, 0, s.component_context->get_input(2));
    s.connect(g_and_2, 1, g_not);
    s.connect(g_out, 0, g_and);
    s.connect(g_out, 1, g_and_2);
    s.connect(s.component_context->get_output(0), 0, g_out);

    SUBCASE("Run 2x1 MUX")
    {
        REQUIRE_EQ(s.component_context->run(0b111), 1);
        REQUIRE_EQ(s.component_context->run(0b110), 0);
        REQUIRE_EQ(s.component_context->run(0b101), 1);
        REQUIRE_EQ(s.component_context->run(0b100), 0);
        REQUIRE_EQ(s.component_context->run(0b011), 1);
        REQUIRE_EQ(s.component_context->run(0b010), 1);
        REQUIRE_EQ(s.component_context->run(0b001), 0);
        REQUIRE_EQ(s.component_context->run(0b000), 0);
    }
}

TEST_CASE("Create a component, grow it")
{
    Scene s { ComponentContext { &s, 2, 1 }, "basic-gate" };
    Node g_and = s.add_node<Gate>(Gate::Type::AND);
    Node g_not = s.add_node<Gate>(Gate::Type::NOT);
    s.connect(g_and, 1, g_not);
    s.connect(g_and, 0, s.component_context->get_input(0));
    s.connect(g_not, 0, s.component_context->get_input(1));
    s.connect(s.component_context->get_output(0), 0, g_and);

    REQUIRE_EQ(s.component_context->run(0b11), 0);
    REQUIRE_EQ(s.component_context->run(0b10), 0);
    REQUIRE_EQ(s.component_context->run(0b01), 1);
    REQUIRE_EQ(s.component_context->run(0b00), 0);

    s.component_context->setup(2, 2);
    REQUIRE(s.connect(s.component_context->get_output(1), 0, g_not));

    REQUIRE_EQ(s.component_context->run(0b11), 0b00);
    REQUIRE_EQ(s.component_context->run(0b10), 0b00);
    REQUIRE_EQ(s.component_context->run(0b01), 0b11);
    REQUIRE_EQ(s.component_context->run(0b00), 0b10);

    s.component_context->setup(2, 1);
    REQUIRE_EQ(s.component_context->run(0b11), 0);
    REQUIRE_EQ(s.component_context->run(0b10), 0);
    lcs_assert(s.component_context->run(0b01) == 1);
    //    REQUIRE_EQ(s.component_context->run(0b01), 1);
    REQUIRE_EQ(s.component_context->run(0b00), 0);
}

TEST_CASE("add-dependency")
{
    Scene s { ComponentContext { &s, 2, 1 }, "component", "author" };
    Node g_and = s.add_node<Gate>(Gate::Type::AND);
    s.connect(s.component_context->get_output(0), 0, g_and);
    s.connect(g_and, 0, s.component_context->get_input(0));
    s.connect(g_and, 1, s.component_context->get_input(1));

    REQUIRE_EQ(s.component_context->run(0b11), 1);
    REQUIRE_EQ(s.component_context->run(0b10), 0);
    REQUIRE_EQ(s.component_context->run(0b01), 0);
    REQUIRE_EQ(s.component_context->run(0b00), 0);
    Scene s2 {};
    s2.add_dependency(std::move(s));
    Node c = s2.add_node<Component>();
    REQUIRE_EQ(s2.get_node<Component>(c)->set_component(0), Error::OK);
    Node i1 = s2.add_node<Input>();
    Node i2 = s2.add_node<Input>();
    Node o  = s2.add_node<Output>();

    REQUIRE(s2.connect(c, 0, i1));
    REQUIRE(s2.connect(c, 1, i2));
    REQUIRE(s2.connect(o, 0, c, 0));
}

TEST_CASE("multi-output-component-dependency")
{
    Scene s { ComponentContext { &s, 2, 3 }, "component", "author" };
    Node g_and = s.add_node<Gate>(Gate::Type::AND);
    Node g_not = s.add_node<Gate>(Gate::Type::NOT);
    Node g_or  = s.add_node<Gate>(Gate::Type::OR);
    s.connect(g_and, 0, s.component_context->get_input(0));
    s.connect(g_and, 1, s.component_context->get_input(1));
    s.connect(g_not, 0, s.component_context->get_input(0));
    s.connect(g_or, 0, s.component_context->get_input(1));
    s.connect(g_or, 1, g_not);

    s.connect(s.component_context->get_output(0), 0, g_and);
    s.connect(s.component_context->get_output(1), 0, g_or);
    s.connect(s.component_context->get_output(2), 0, g_not);
    REQUIRE_EQ(s.component_context->run(0b11), 0b011);
    REQUIRE_EQ(s.component_context->run(0b10), 0b110);
    REQUIRE_EQ(s.component_context->run(0b01), 0b000);
    REQUIRE_EQ(s.component_context->run(0b00), 0b110);

    Scene s2 {};
    s2.add_dependency(std::move(s));
    Node c = s2.add_node<Component>();
    REQUIRE_EQ(s2.get_node<Component>(c)->set_component(0), Error::OK);
    Node i1 = s2.add_node<Input>();
    Node i2 = s2.add_node<Input>();
    Node o1 = s2.add_node<Output>();
    Node o2 = s2.add_node<Output>();
    Node o3 = s2.add_node<Output>();

    REQUIRE(s2.connect(c, 0, i1));
    REQUIRE(s2.connect(c, 1, i2));
    REQUIRE(s2.connect(o1, 0, c, 0));
    REQUIRE(s2.connect(o2, 0, c, 1));
    REQUIRE(s2.connect(o3, 0, c, 2));

    s2.get_node<Input>(i1)->set(true);
    s2.get_node<Input>(i2)->set(true);
    REQUIRE_EQ(s2.get_node<Output>(o1)->get(), TRUE);
    REQUIRE_EQ(s2.get_node<Output>(o2)->get(), TRUE);
    REQUIRE_EQ(s2.get_node<Output>(o3)->get(), FALSE);

    s2.get_node<Input>(i1)->set(true);
    s2.get_node<Input>(i2)->set(false);
    REQUIRE_EQ(s2.get_node<Output>(o1)->get(), FALSE);
    REQUIRE_EQ(s2.get_node<Output>(o2)->get(), TRUE);
    REQUIRE_EQ(s2.get_node<Output>(o3)->get(), TRUE);

    s2.get_node<Input>(i1)->set(false);
    s2.get_node<Input>(i2)->set(true);
    REQUIRE_EQ(s2.get_node<Output>(o1)->get(), FALSE);
    REQUIRE_EQ(s2.get_node<Output>(o2)->get(), FALSE);
    REQUIRE_EQ(s2.get_node<Output>(o3)->get(), FALSE);

    s2.get_node<Input>(i1)->set(false);
    s2.get_node<Input>(i2)->set(false);
    REQUIRE_EQ(s2.get_node<Output>(o1)->get(), FALSE);
    REQUIRE_EQ(s2.get_node<Output>(o2)->get(), TRUE);
    REQUIRE_EQ(s2.get_node<Output>(o3)->get(), TRUE);
}
