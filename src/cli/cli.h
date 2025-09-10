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
        std::function<Error(Ref<Scene>, const std::string& arg)> _cmd,
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
    std::function<Error(Ref<Scene>, const std::string& arg)> cmd;
    std::array<char, 128> msg { 0 };
};
extern std::array<Command, 34> root;

} // namespace lcs::cli
