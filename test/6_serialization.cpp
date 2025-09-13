#include <doctest.h>
#include "common.h"
#include "core.h"
#include "test_util.h"

using namespace ic;
TEST_CASE("parse-scene")
{
    Scene s { "parse-scene-subnodes" };
    Node g_or  = s.add_node<Gate>(Gate::Type::OR);
    Node g_and = s.add_node<Gate>(Gate::Type::AND);
    Node i1    = s.add_node<Input>();
    Node i2    = s.add_node<Input>();
    Node i3    = s.add_node<Input>();
    Node o     = s.add_node<Output>();

    s.connect(g_or, 0, i1);
    s.connect(g_or, 1, i2);
    s.connect(g_and, 0, i3);
    s.connect(g_and, 1, i1);
    s.connect(o, 0, g_or);
    s.get_node<Input>(i1)->set(true);
    s.get_node<Input>(i2)->set(false);
    s.get_node<Input>(i3)->set(true);

    std::vector<uint8_t> v;
    REQUIRE_EQ(s.write_to(v), Error::OK);
    REQUIRE(!v.empty());
}

TEST_CASE("parse-scene-with-different-input-types")
{
    Scene s { "different-input-types" };

    Node g_and = s.add_node<Gate>(Gate::Type::AND);
    Node i1    = s.add_node<Input>();
    Node i2    = s.add_node<Input>();
    Node o     = s.add_node<Output>();
    s.get_node<Input>(i1)->set_freq(3);

    s.connect(g_and, 0, i2);
    s.connect(g_and, 1, i1);
    s.connect(o, 0, g_and);
    s.get_base(i1)->move({ 1, 3 });

    std::vector<uint8_t> v;
    REQUIRE_EQ(s.write_to(v), Error::OK);
    REQUIRE(!v.empty());
}

TEST_CASE("save-load-compare")
{
    Scene s { "load-and-compare" };
    Node i1 = s.add_node<Input>();
    Node o  = s.add_node<Output>();
    s.get_node<Input>(i1)->set(true);
    s.connect(o, 0, i1);

    std::vector<uint8_t> data { 0 };
    REQUIRE_EQ(s.write_to(data), Error::OK);
    Scene s_loaded;
    REQUIRE_EQ(s_loaded.read_from(data), Error::OK);
    std::vector<uint8_t> data_loaded { 0 };
    REQUIRE_EQ(s_loaded.write_to(data_loaded), Error::OK);

    REQUIRE_EQ(s_loaded.get_node<Output>(o)->get(), State::TRUE);

    REQUIRE_EQ(data.size(), data_loaded.size());
    REQUIRE(scene_cmp(s, s_loaded));
}

TEST_CASE("save-load-compare-runs")
{
    // Taken from TEST_CASE("Full Adder")
    Scene s { "complicated-scene" };
    _create_full_adder_io(s);
    _create_full_adder(s);

    std::vector<uint8_t> data;
    REQUIRE_EQ(s.write_to(data), Error::OK);
    Scene s_loaded;
    REQUIRE_EQ(s_loaded.read_from(data), Error::OK);

    s_loaded.get_node<Input>(a)->set(true);
    s_loaded.get_node<Input>(b)->set(false);
    s_loaded.get_node<Input>(c_in)->set(false);
    REQUIRE_EQ(s_loaded.get_node<Output>(sum)->get(), State::TRUE);
    REQUIRE_EQ(s_loaded.get_node<Output>(c_out)->get(), State::FALSE);

    s_loaded.get_node<Input>(b)->set(true);
    REQUIRE_EQ(s_loaded.get_node<Output>(sum)->get(), State::FALSE);
    REQUIRE_EQ(s_loaded.get_node<Output>(c_out)->get(), State::TRUE);

    s_loaded.get_node<Input>(c_in)->set(true);
    REQUIRE_EQ(s_loaded.get_node<Output>(sum)->get(), State::TRUE);
    REQUIRE_EQ(s_loaded.get_node<Output>(c_out)->get(), State::TRUE);
}

TEST_CASE("save-component-reload-run")
{
    // Taken from "Create a 2x1 MUX component"
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

    REQUIRE_EQ(s.component_context->run(0b111), 1);
    REQUIRE_EQ(s.component_context->run(0b110), 0);
    REQUIRE_EQ(s.component_context->run(0b101), 1);
    REQUIRE_EQ(s.component_context->run(0b100), 0);
    REQUIRE_EQ(s.component_context->run(0b011), 1);
    REQUIRE_EQ(s.component_context->run(0b010), 1);
    REQUIRE_EQ(s.component_context->run(0b001), 0);
    REQUIRE_EQ(s.component_context->run(0b000), 0);

    std::vector<uint8_t> data;
    REQUIRE_EQ(s.write_to(data), Error::OK);
    Scene s_loaded;
    REQUIRE_EQ(s_loaded.read_from(data), Error::OK);
    std::vector<uint8_t> data_loaded;
    REQUIRE_EQ(s_loaded.write_to(data_loaded), Error::OK);
    REQUIRE_EQ(s_loaded.component_context->run(0b111), 1);
    REQUIRE_EQ(s_loaded.component_context->run(0b110), 0);
    REQUIRE_EQ(s_loaded.component_context->run(0b101), 1);
    REQUIRE_EQ(s_loaded.component_context->run(0b100), 0);
    REQUIRE_EQ(s_loaded.component_context->run(0b011), 1);
    REQUIRE_EQ(s_loaded.component_context->run(0b010), 1);
    REQUIRE_EQ(s_loaded.component_context->run(0b001), 0);
    REQUIRE_EQ(s_loaded.component_context->run(0b000), 0);
}

TEST_CASE("save-component-load-to-scene")
{
    std::vector<uint8_t> data;
    {
        Scene s { ComponentContext { &s, 2, 1 }, "XOR", "Author",
            "Description" };
        Node g_xor = s.add_node<Gate>(Gate::Type::XOR);
        s.connect(g_xor, 0, s.component_context->get_input(0));
        s.connect(g_xor, 1, s.component_context->get_input(1));
        s.connect(s.component_context->get_output(0), 0, g_xor);
        REQUIRE_EQ(s.write_to(data), Error::OK);
        REQUIRE(fs::write(
            fs::LIBRARY / (base64_encode(s.to_dependency()) + ".ic"), data));

        Scene s2 { "DependencyScene", "Author", "Description" };
        s2.add_dependency(std::move(s));
        Node c = s2.add_node<Component>();
        REQUIRE_EQ(s2.get_node<Component>(c)->set_component(0), Error::OK);
        Node i1 = s2.add_node<Input>();
        Node i2 = s2.add_node<Input>();
        Node o  = s2.add_node<Output>();
        REQUIRE(s2.connect(o, 0, c, 0));
        REQUIRE(s2.connect(c, 0, i1));
        REQUIRE(s2.connect(c, 1, i2));
        s2.get_node<Input>(i1)->set(true);
        REQUIRE_EQ(s2.write_to(data), Error::OK);
    }

    Scene s3;
    REQUIRE_EQ(s3.read_from(data), Error::OK);
    REQUIRE_EQ(s3.get_node<Output>(Node { 0, Node::OUTPUT })->get(), TRUE);
}
