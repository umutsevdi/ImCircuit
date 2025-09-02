#pragma once
/*******************************************************************************
 * \file
 * File: cli/cli.h
 * Created: 09/03/25
 * Author: Umut Sevdi
 * Description: Command line parser and interactive shell
 *
 * Project: umutsevdi/logic-circuit-simulator-2
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/
#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include "core.h"
namespace lcs::cli {
int parse_args(int argc, char** argv);
int run(void);

enum Type : uint8_t {
    NONE,
    STR,
    INT,
    BOOL,
    NODE,
    NODE_INT_INT,
    NODE_INT_NODE_INT
};
class Command {

public:
    Command(const std::string& _name, const std::string& _desc,
        std::function<Error(NRef<Scene>, const std::string& arg)> _cmd,
        Type _type = NONE, bool _optional = false);

    Command(Command&&)                 = default;
    Command(const Command&)            = default;
    Command& operator=(Command&&)      = default;
    Command& operator=(const Command&) = default;
    ~Command()                         = default;

    bool is_matching(const std::string& line, std::string& arg) const;

    std::string name;
    Type type     = NONE;
    bool optional = false;
    std::function<Error(NRef<Scene>, const std::string& arg)> cmd;
    std::array<char, 128> msg { 0 };
};
extern std::array<Command, 34> root;

Error _add_component(NRef<Scene>, const std::string& arg);
Error _add_gate_and(NRef<Scene>, const std::string& arg);
Error _add_gate_nand(NRef<Scene>, const std::string& arg);
Error _add_gate_nor(NRef<Scene>, const std::string& arg);
Error _add_gate_not(NRef<Scene>, const std::string& arg);
Error _add_gate_or(NRef<Scene>, const std::string& arg);
Error _add_gate_xnor(NRef<Scene>, const std::string& arg);
Error _add_gate_xor(NRef<Scene>, const std::string& arg);
Error _add_input(NRef<Scene>, const std::string& arg);
Error _add_output(NRef<Scene>, const std::string& arg);
Error _add_timer(NRef<Scene>, const std::string& arg);
Error _close(NRef<Scene>, const std::string& arg);
Error _connect(NRef<Scene>, const std::string& arg);
Error _disconnect(NRef<Scene>, const std::string& arg);
Error _help(NRef<Scene>, const std::string& arg);
Error _include(NRef<Scene>, const std::string& arg);
Error _list_all(NRef<Scene>, const std::string& arg);
Error _list_component(NRef<Scene>, const std::string& arg);
Error _list_gate(NRef<Scene>, const std::string& arg);
Error _list_input(NRef<Scene>, const std::string& arg);
Error _list_rel(NRef<Scene>, const std::string& arg);
Error _list_output(NRef<Scene>, const std::string& arg);
Error _move(NRef<Scene>, const std::string& arg);
Error _new(NRef<Scene>, const std::string& arg);
Error _open(NRef<Scene>, const std::string& arg);
Error _remove(NRef<Scene>, const std::string& arg);
Error _save_as(NRef<Scene>, const std::string& arg);
Error _save(NRef<Scene>, const std::string& arg);
Error _set_author(NRef<Scene>, const std::string& arg);
Error _set_desc(NRef<Scene>, const std::string& arg);
Error _set_name(NRef<Scene>, const std::string& arg);
Error _show_rel(NRef<Scene>, const std::string& arg);
Error _show_node(NRef<Scene>, const std::string& arg);
Error _toggle(NRef<Scene>, const std::string& arg);
} // namespace lcs::cli
