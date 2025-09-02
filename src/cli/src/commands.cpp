#include <charconv>
#include "cli.h"
#include "common.h"

namespace lcs::cli {

Command::Command(const std::string& _name, const std::string& _desc,
    std::function<Error(NRef<Scene>, const std::string& arg)> _cmd, Type _value,
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
