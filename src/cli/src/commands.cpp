#include <string>
#include "cli.h"
#include "common.h"
#include "core.h"
#include "errors.h"
#define expect_scene(scene)                                                    \
    if (scene == nullptr)                                                      \
        return ERROR(Error::NO_SCENE);

namespace lcs::cli {

Command::Command(const std::string& _name, const std::string& _desc,
    std::function<Error(Ref<Scene>, const std::string& arg)> _cmd, Type _value,
    bool _optional)
    : name { _name }
    , type { _value }
    , optional { _optional }
    , cmd { _cmd }
{
    if (type != NONE) {
        std::snprintf(msg.data(), msg.max_size() - 1, "%-15s %19s%c ~ %s",
            name.c_str(), to_str<Type>(type), optional ? '?' : ' ',
            _desc.c_str());
    } else {
        std::snprintf(msg.data(), msg.max_size() - 1, "%-15s %*s ~ %s",
            name.c_str(), 20, " ", _desc.c_str());
    }
};

bool Command::is_matching(const std::string& line, std::string& arg) const
{
    if (line.find(name) != 0) {
        return false;
    }
    size_t arg_start = line.find(' ', name.size());
    if (arg_start == std::string::npos) {
        arg_start = name.size();
    } else {
        arg_start++;
    }
    arg = line.substr(arg_start);
    if (arg.empty() && type != Type::NONE && !optional) {
        L_ERROR("Function expectes an argument of type %s. "
                "Found null",
            to_str<Type>(type));
        return false;
    }
    return true;
}

static Error as(const std::string& data, int& value, bool required = true)
{
    if (data.empty()) {
        return required ? ERROR(Error::NO_ARGUMENT) : Error::OK;
    }
    try {
        value = std::stoi(data);
        return Error::OK;
    } catch (const std::exception& e) {
        return ERROR(Error::INVALID_ARGUMENT);
    }
}

static Error as(const std::string& data, bool& value, bool required = true)
{
    if (data.empty()) {
        return required ? ERROR(Error::NO_ARGUMENT) : Error::OK;
    }
    if (data == "true") {
        value = true;
    } else if (data == "false") {
        value = false;
    } else {
        return ERROR(Error::INVALID_ARGUMENT);
    }
    return Error::OK;
}

static Error as(const std::string& data, Node& value, bool required = true)
{
    if (data.empty()) {
        return required ? ERROR(Error::NO_ARGUMENT) : Error::OK;
    }
    size_t symbol = data.find('@');
    if (symbol == std::string::npos) {
        return ERROR(Error::INVALID_ARGUMENT);
    }
    int idx = 0;
    if (Error err = as(data.substr(symbol + 1), idx); err != Error ::OK) {
        return err;
    }
    std::string type = data.substr(0, symbol);
    if (type == "Gate") {
        value.type = Node::GATE;
    } else if (type == "Component") {
        value.type = Node::COMPONENT;
    } else if (type == "Input") {
        value.type = Node::INPUT;
    } else if (type == "Output") {
        value.type = Node::OUTPUT;
    } else if (type == "Component Output") {
        value.type = Node::COMPONENT_OUTPUT;
    } else if (type == "Component Input") {
        value.type = Node::COMPONENT_INPUT;
    } else {
        return ERROR(Error::INVALID_NODE);
    }
    value.index = idx;
    return Error::OK;
}

Error print_node(Node node, Ref<Gate> n)
{
    if (n == nullptr) {
        return ERROR(Error::NODE_NOT_FOUND);
    };
    std::stringstream input_info {};
    for (size_t i = 0; i < n->inputs.size(); i++) {
        input_info << "[" << i << ":"
                   << (n->inputs[i] != 0 ? std::to_string(n->inputs[i])
                                         : "empty")
                   << ']';
    }
    std::stringstream output_info {};
    for (auto& out : n->output) {
        output_info << out << ' ';
    }
    L_INFO("Gate@%zu: type:%s connected:%s position:(%d,%d) inputs: {%s}, "
           "outputs: {%s}",
        node.index, to_str<Gate::Type>(n->type()),
        n->is_connected() ? "true" : "false", n->point().x, n->point().y,
        input_info.str().c_str(), output_info.str().c_str());
    return Error::OK;
}

Error print_node(Node node, Ref<Input> n)
{
    if (n == nullptr) {
        return ERROR(Error::NODE_NOT_FOUND);
    }
    std::stringstream output_info {};
    for (auto& out : n->output) {
        output_info << out << ' ';
    }
    if (n->is_timer()) {
        L_INFO("Timer@%zu: freq: %.2f, is_connected: %s, position: "
               "(%d, %d), output: {%s}",
            node.index, n->freq() / 10.f, n->is_connected() ? "true" : "false",
            n->point().x, n->point().y, output_info.str().c_str());
    } else {
        L_INFO("Input@%zu: value: %s, is_connected: %s, position: "
               "(%d, %d), output: {%s}",
            node.index, to_str<State>(n->get()),
            n->is_connected() ? "true" : "false", n->point().x, n->point().y,
            output_info.str().c_str());
    }
    return Error::OK;
}

Error print_node(Node node, Ref<Output> n)
{
    if (n == nullptr) {
        return ERROR(Error::NODE_NOT_FOUND);
    }
    L_INFO("Output@%zu: value: %s, is_connected: %s position: (%d, %d)",
        node.index, to_str<State>(n->get()),
        n->is_connected() ? "true" : "false", n->point().x, n->point().y);
    return Error::OK;
}

Error print_node(Node node, Ref<Component> n)
{
    return Error::OK; /** TODO implement*/
}

Error print_node(Node node, Ref<ComponentContext> n)
{
    return Error::OK; /** TODO implement*/
}

Error _add_component(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);

    return Error::OK;
}

static inline Error _parse_gate_size(
    Ref<Scene> scene, const std::string& arg, int& amount)
{
    expect_scene(scene);
    int max_in = 2;
    if (Error err = as(arg, max_in, false); err != Error ::OK) {
        return err;
    }
    if (max_in <= 0) {
        return ERROR(Error::INVALID_ARGUMENT);
    }
    amount = max_in;
    return Error::OK;
}

Error _add_gate_and(Ref<Scene> scene, const std::string& arg)
{
    int amount = 2;
    if (Error err = _parse_gate_size(scene, arg, amount); err != Error::OK) {
        return err;
    };
    scene->add_node<Gate>(Gate::Type::AND, amount);
    return Error::OK;
}

Error _add_gate_nand(Ref<Scene> scene, const std::string& arg)
{
    int amount = 2;
    if (Error err = _parse_gate_size(scene, arg, amount); err != Error::OK) {
        return err;
    };
    scene->add_node<Gate>(Gate::Type::NAND, amount);
    return Error::OK;
}

Error _add_gate_nor(Ref<Scene> scene, const std::string& arg)
{
    int amount = 2;
    if (Error err = _parse_gate_size(scene, arg, amount); err != Error::OK) {
        return err;
    };
    scene->add_node<Gate>(Gate::Type::NOR, amount);
    return Error::OK;
}

Error _add_gate_not(Ref<Scene> scene, const std::string&)
{
    expect_scene(scene);
    scene->add_node<Gate>(Gate::Type::NOT);
    return Error::OK;
}

Error _add_gate_or(Ref<Scene> scene, const std::string& arg)
{
    int amount = 2;
    if (Error err = _parse_gate_size(scene, arg, amount); err != Error::OK) {
        return err;
    };
    scene->add_node<Gate>(Gate::Type::OR, amount);
    return Error::OK;
}

Error _add_gate_xnor(Ref<Scene> scene, const std::string& arg)
{
    int amount = 2;
    if (Error err = _parse_gate_size(scene, arg, amount); err != Error::OK) {
        return err;
    };
    scene->add_node<Gate>(Gate::Type::XNOR, amount);
    return Error::OK;
}

Error _add_gate_xor(Ref<Scene> scene, const std::string& arg)
{
    int amount = 2;
    if (Error err = _parse_gate_size(scene, arg, amount); err != Error::OK) {
        return err;
    };
    scene->add_node<Gate>(Gate::Type::XOR, amount);
    return Error::OK;
}

Error _add_input(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    bool value = false;
    if (Error err = as(arg, value, false); err != Error ::OK) {
        return err;
    }
    scene->get_node<Input>(scene->add_node<Input>())->set(value);
    return Error::OK;
}

Error _add_output(Ref<Scene> scene, const std::string&)
{
    expect_scene(scene);
    scene->add_node<Output>();
    return Error::OK;
}

Error _add_timer(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    int freq = 10;
    if (Error err = as(arg, freq, false); err != Error ::OK) {
        return err;
    }
    scene->add_node<Input>(freq);
    return Error::OK;
}

Error _close(Ref<Scene>, const std::string&) { return tabs::close(); }

Error _connect(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    Node from, to;
    int from_sock, to_sock;

    size_t from_node_end = arg.find(' ');
    if (from_node_end == std::string::npos) {
        return ERROR(Error::INVALID_NODE);
    }
    Error err = as(arg.substr(0, from_node_end), from);
    if (err) {
        return err;
    }
    size_t from_sock_end = arg.find(' ', from_node_end + 1);
    if (from_sock_end == std::string::npos) {
        return ERROR(Error::INVALID_ARGUMENT);
    }
    err = as(arg.substr(from_node_end + 1, from_sock_end - from_node_end - 1),
        from_sock);
    if (err) {
        return err;
    }
    size_t to_node_end = arg.find(' ', from_sock_end + 1);
    if (to_node_end == std::string::npos) {
        return ERROR(Error::INVALID_NODE);
    }
    err = as(
        arg.substr(from_sock_end + 1, to_node_end - from_sock_end - 1), to);
    if (err) {
        return err;
    }
    err = as(arg.substr(to_node_end + 1), to_sock);
    if (err) {
        return err;
    }

    return scene->connect_with_id(0, to, to_sock, from, from_sock);
}

Error _disconnect(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    int id;
    if (Error err = as(arg, id); err != Error ::OK) {
        return err;
    }
    return scene->disconnect(id);
}

Error _help(Ref<Scene>, const std::string&)
{
    L_INFO(APPNAME " shell provides the following commands:");
    L_INFO("Argument types are enclosed in parentheses, and a trailing \"?\" "
           "indicates that the argument is optional.");
    for (const auto& t : root) {
        L_INFO(t.msg.data());
    }
    return Error::OK;
};

Error _include(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    if (arg.empty()) {
        return ERROR(Error::NO_ARGUMENT);
    }
    return scene->add_dependency(arg);
}

Error _list_component(Ref<Scene> scene, const std::string&)
{
    expect_scene(scene);
    for (size_t i = 0; i < scene->_components.size(); i++) {
        if (!scene->_components[i].is_null()) {
            const auto& g = scene->_components[i];
            L_INFO(
                " > Component@%zu | source: %s, value: %s, is_connected: %s, "
                "position: (%d, %d)",
                i, scene->dependencies()[g.dep_idx].to_dependency().c_str(),
                to_str<State>(g.get()), g.is_connected() ? "true" : "false",
                g.point().x, g.point().y);
        }
    }
    return Error::OK;
}

Error _list_gate(Ref<Scene> scene, const std::string&)
{
    expect_scene(scene);
    for (size_t i = 0; i < scene->_gates.size(); i++) {
        if (!scene->_gates[i].is_null()) {
            const auto& g = scene->_gates[i];
            L_INFO(" > Gate@%zu   | type: %s, value: %s, is_connected: %s, "
                   "position: (%d, %d)",
                i, to_str<Gate::Type>(g.type()), to_str<State>(g.get()),
                g.is_connected() ? "true" : "false", g.point().x, g.point().y);
        }
    }
    return Error::OK;
}

Error _list_input(Ref<Scene> scene, const std::string&)
{
    expect_scene(scene);
    for (size_t i = 0; i < scene->_inputs.size(); i++) {
        if (!scene->_inputs[i].is_null()) {
            const auto& g = scene->_inputs[i];
            if (g.is_timer()) {
                L_INFO(
                    " > Timer@%zu  | freq: %.2f, is_connected: %s, position: "
                    "(%d, %d)",
                    i, g.freq() / 10.f, g.is_connected() ? "true" : "false",
                    g.point().x, g.point().y);
            } else {
                L_INFO(" > Input@%zu  | value: %s, is_connected: %s, position: "
                       "(%d, %d)",
                    i, to_str<State>(g.get()),
                    g.is_connected() ? "true" : "false", g.point().x,
                    g.point().y);
            }
        }
    }
    return Error::OK;
}

Error _list_output(Ref<Scene> scene, const std::string&)
{
    expect_scene(scene);
    for (size_t i = 0; i < scene->_outputs.size(); i++) {
        if (!scene->_outputs[i].is_null()) {
            const auto& g = scene->_outputs[i];
            L_INFO(" > Output@%zu | value: %s, is_connected: %s "
                   "position: (%d, %d)",
                i, to_str<State>(g.get()), g.is_connected() ? "true" : "false",
                g.point().x, g.point().y);
        }
    }
    return Error::OK;
}

Error _list_rel(Ref<Scene> scene, const std::string&)
{
    for (const auto& [k, v] : scene->_relations) {
        L_INFO("> %5zu | %6s@%zu[%d] -[%-5s]-> %6s@%zu[%d]", k,
            to_str<Node::Type>(v.from_node.type), v.from_node.index,
            v.from_sock, to_str<State>(v.value),
            to_str<Node::Type>(v.to_node.type), v.to_node.index, v.to_sock);
    }
    return Error::OK;
}

Error _list_all(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    _list_gate(scene, arg);
    _list_input(scene, arg);
    _list_output(scene, arg);
    _list_component(scene, arg);
    return Error::OK;
}

Error _move(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    if (arg.empty()) {
        return ERROR(Error::NO_ARGUMENT);
    }
    size_t x_start = arg.find(' ');
    if (x_start == std::string::npos) {
        return ERROR(Error::NO_ARGUMENT);
    }
    size_t y_start = arg.find(' ', x_start + 1);
    if (y_start == std::string::npos) {
        return ERROR(Error::NO_ARGUMENT);
    }
    Node n;
    int x = 0, y = 0;
    Error err = Error::OK;
    if (err = as(arg.substr(0, x_start), n); err != Error::OK) {
        return err;
    }
    if (err = as(arg.substr(x_start, y_start - x_start - 1), x);
        err != Error::OK) {
        return err;
    }
    if (err = as(arg.substr(y_start + 1), y); err != Error::OK) {
        return err;
    }
    auto node = scene->get_base(n);
    if (node == nullptr) {
        return ERROR(Error::NODE_NOT_FOUND);
    }
    node->move({ static_cast<int16_t>(x), static_cast<int16_t>(y) });
    return Error::OK;
}

Error _new(Ref<Scene> scene, const std::string& arg)
{
    if (scene != nullptr) {
        return ERROR(Error::ALREADY_ACTIVE_SCENE);
    }
    if (arg.size() > scene->name().max_size()) {
        return ERROR(Error::INVALID_STRING);
    }
    tabs::create(arg, "local", "", 1);
    return Error::OK;
}

Error _open(Ref<Scene> scene, const std::string& arg)
{
    if (scene != nullptr) {
        return ERROR(Error::ALREADY_ACTIVE_SCENE);
    }
    if (arg.empty()) {
        return ERROR(Error::INVALID_ARGUMENT);
    }
    return tabs::open(arg);
}

Error _remove(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    Node node;
    if (Error err = as(arg, node); err != Error ::OK) {
        return err;
    }
    return scene->remove_node(node);
}

Error _save_as(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    if (arg.empty()) {
        return ERROR(Error::NO_ARGUMENT);
    }
    return tabs::save_as(arg);
}

Error _save(Ref<Scene> scene, const std::string&)
{
    expect_scene(scene);
    return tabs::save();
}

Error _set_author(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    if (arg.empty()) {
        return ERROR(Error::NO_ARGUMENT);
    }
    return scene->set_author(arg);
}

Error _set_desc(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    if (arg.empty()) {
        return ERROR(Error::NO_ARGUMENT);
    }
    return scene->set_description(arg);
}

Error _set_name(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    if (arg.empty()) {
        return ERROR(Error::NO_ARGUMENT);
    }
    return scene->set_name(arg);
}

Error _show_rel(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    int rel = 0;
    if (Error err = as(arg, rel); err != Error ::OK) {
        return err;
    }
    auto r = scene->get_rel(rel);
    if (r == nullptr) {
        return ERROR(Error::REL_NOT_FOUND);
    }
    L_INFO("> %5zu | %6s@%zu[%d] -[%-5s]-> %6s@%zu[%d]", rel,
        to_str<Node::Type>(r->from_node.type), r->from_node.index, r->from_sock,
        to_str<State>(r->value), to_str<Node::Type>(r->to_node.type),
        r->to_node.index, r->to_sock);
    return Error::OK;
}

Error _show_node(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    Node node;
    if (Error err = as(arg, node); err != Error ::OK) {
        return err;
    }
    switch (node.type) {
    case Node::GATE: print_node(node, scene->get_node<Gate>(node)); break;
    case Node::INPUT: print_node(node, scene->get_node<Input>(node)); break;
    case Node::OUTPUT: print_node(node, scene->get_node<Output>(node)); break;
    default: break;
    }

    return Error::OK;
}

Error _toggle(Ref<Scene> scene, const std::string& arg)
{
    expect_scene(scene);
    Node node;
    if (Error err = as(arg, node); err != Error ::OK) {
        return err;
    }
    if (node.type == Node::Type::INPUT) {
        auto n = scene->get_node<Input>(node);
        if (n == nullptr) {
            return ERROR(Error::NODE_NOT_FOUND);
        }
        n->toggle();
    }
    return Error::OK;
}

std::array<Command, 34> root {
    Command {
        "add component", "Add a component to the scene.", _add_component, STR },
    { "add gate AND", "Add an AND gate.", _add_gate_and, INT, true },
    { "add gate NAND", "Add a NAND gate.", _add_gate_nand, INT, true },
    { "add gate NOR", "Add a NOR gate.", _add_gate_nor, INT, true },
    { "add gate NOT", "Add a NOT gate.", _add_gate_not },
    { "add gate OR", "Add an OR gate.", _add_gate_or, INT, true },
    { "add gate XNOR", "Add an XNOR gate.", _add_gate_xnor, INT, true },
    { "add gate XOR", "Add an XOR gate.", _add_gate_xor, INT, true },
    { "add input", "Add an input.", _add_input, BOOL, true },
    { "add output", "Add an output.", _add_output },
    { "add timer", "Add a timer.", _add_timer, INT, true },
    { "close", "Close the existing scene.", _close },
    { "connect", "Connect two nodes.", _connect, NODE_INT_NODE_INT },
    { "disconnect", "Severe a connection.", _disconnect, INT },
    { "help", "Display information about the shell.", _help },
    { "include ", "Import a dependency to the active scene.", _include, STR },
    { "list component", "List all components.", _list_component },
    { "list gate", "List all logic gates.", _list_gate },
    { "list input", "List all inputs and timers.", _list_input },
    { "list output", "List all outputs.", _list_output },
    { "list rel", "List all conections.", _list_rel },
    { "list", "List all objects.", _list_all },
    { "Move", "Move selected node in x y coordinates.", _move, NODE_INT_INT },
    { "new", "Create a new scene.", _new, STR, true },
    { "open", "Open a saved scene.", _open, STR },
    { "remove", "Delete selected node.", _remove, NODE },
    { "save as", "Save active scene to new path.", _save_as, STR },
    { "save", "Save existing scene.", _save },
    { "set author", "Set author of the scene.", _set_author, STR },
    { "set desc", "Set description of the scene.", _set_desc, STR },
    { "set name", "Set name of the scene.", _set_name, STR },
    { "show rel", "Display information about selected connection.", _show_rel,
        INT },
    { "show node", "Display information about selected node.", _show_node,
        NODE },
    { "toggle", "Toggle selected node.", _toggle, NODE },
};

} // namespace lcs::cli
