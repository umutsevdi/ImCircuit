#include <doctest.h>
#include "common.h"
#include "core.h"

using namespace lcs;

TEST_CASE("Save a scene and load")
{
    size_t s_handle = tabs::create("subnode", "author", "", 1);
    auto s          = tabs::active(s_handle);
    Node g_or       = s->add_node<Gate>(Gate::Type::OR);
    Node g_and      = s->add_node<Gate>(Gate::Type::AND);
    Node v1         = s->add_node<Input>();
    Node v2         = s->add_node<Input>();
    Node v3         = s->add_node<Input>();
    Node o          = s->add_node<Output>();

    s->connect(g_or, 0, v1);
    s->connect(g_or, 1, v2);
    s->connect(g_and, 0, v3);
    s->connect(g_and, 1, v1);
    s->connect(o, 0, g_or);
    REQUIRE_EQ(
        tabs::save_as(lcs::fs::LIBRARY / "test.lcs", s_handle), Error::OK);
    REQUIRE_EQ(tabs::close(s_handle), Error::OK);
    REQUIRE_EQ(tabs::open(lcs::fs::LIBRARY / "test.lcs"), Error::OK);
    REQUIRE_EQ(tabs::close(), Error::OK);
}

TEST_CASE("Try Invalid Actions")
{
    REQUIRE_NE(tabs::close(42), Error::OK);
    REQUIRE_NE(tabs::open(fs::LIBRARY / "NOT_FOUND.SCENE"), Error::OK);
    fs::write(fs::LIBRARY / "INVALID_SCENE.lcs", "INVALID_BYTES");
    REQUIRE_NE(tabs::open(fs::LIBRARY / "INVALID_SCENE.lcs"), Error::OK);
}

TEST_CASE("Notify")
{
    tabs::create("UpdatedScene", "Author", "", 1);
    REQUIRE_FALSE(tabs::is_saved());
    REQUIRE(tabs::is_changed());
    REQUIRE_FALSE(tabs::is_changed());
    REQUIRE_EQ(tabs::close(), Error::OK);
}

TEST_CASE("Create tabs and then close")
{
    for (size_t i = 0; i < 10; i++) {
        tabs::create(std::to_string(i), "Author", "", i);
    }
    for (size_t i = 0; i < 10; i++) {
        REQUIRE_EQ(tabs::close(), Error::OK);
    }
    REQUIRE_EQ(tabs::active(), nullptr);
}

TEST_CASE("Open save close reopen")
{
    size_t id = tabs::create("scene", "Author", "", 0);
    REQUIRE_EQ(tabs::save_as(fs::LIBRARY / "scene.lcs", id), Error::OK);
    REQUIRE_EQ(tabs::close(), Error::OK);
    REQUIRE_EQ(tabs::active(), nullptr);
    REQUIRE_EQ(tabs::open(fs::LIBRARY / "scene.lcs"), Error::OK);
    REQUIRE_NE(tabs::active(), nullptr);
    REQUIRE_EQ(tabs::close(), Error::OK);
}

TEST_CASE("Get result from saved scene")
{
    size_t id      = tabs::create("example scene", "Author", "", 0);
    auto scene     = tabs::active();
    Node input     = scene->add_node<Input>();
    Node input2    = scene->add_node<Input>();
    Node dead_node = scene->add_node<Output>();
    Node gate      = scene->add_node<Gate>(Gate::Type::OR);
    Node output    = scene->add_node<Output>();
    Error _        = scene->remove_node(dead_node);
    scene->connect(gate, 0, input);
    scene->connect(gate, 1, input2);
    scene->connect(output, 0, gate);
    scene->get_node<Input>(input)->set(true);

    REQUIRE_EQ(scene->get_node<Output>(output)->get(), State::TRUE);
    REQUIRE_EQ(tabs::save_as(fs::LIBRARY / "or_scene.lcs", id), Error::OK);
    REQUIRE_EQ(tabs::close(), Error::OK);
    REQUIRE_EQ(tabs::open(fs::LIBRARY / "or_scene.lcs"), Error::OK);
    auto scene2 = tabs::active();
    REQUIRE_EQ(scene2->get_node<Output>(output)->get(), State::TRUE);
    REQUIRE_EQ(tabs::close(), Error::OK);
}
