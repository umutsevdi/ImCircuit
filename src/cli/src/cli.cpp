#include <replxx.hxx>

#include "cli.h"
#include "common.h"
#include "core.h"

namespace ic {
template <> const char* to_str<cli::Type>(cli::Type v)
{
    switch (v) {
    case ic::cli::NONE: return "<none>";
    case ic::cli::STR: return "<string>";
    case ic::cli::INT: return "<int>"; ;
    case ic::cli::BOOL: return "<bool>";
    case ic::cli::NODE: return "<node>";
    case ic::cli::NODE_INT_INT: return "<node int int>";
    case ic::cli::NODE_INT_NODE_INT: return "<node int node int>";
    }
}
namespace cli {
    using namespace replxx;
    static void _print_help(void);

    int parse_args(int argc, char** argv)
    {
        bool gui = true;
        std::vector<std::string> args;
        args.reserve(argc - 1);
        for (int i = 1; i < argc; i++) {
            args.push_back(argv[i]);
        }

        for (auto& arg : args) {
            if (arg == "-i" || arg == "--interactive") {
                gui = false;
            } else if (arg == "-h" || arg == "--help") {
                _print_help();
                exit(0);
            } else if (arg == "-v" || arg == "--version") {
                printf(APPPKG "." APPVERSION "." APPOS "." APPBUILD "\r\n");
                exit(0);
            } else if (arg == "-V" || arg == "--verbose") {
                fs::is_verbose = true;
            } else {
                std::filesystem::path filepath { arg };
                if (!(filepath.has_extension()
                        && filepath.extension() == ".ic")) {
                    return ERROR(Error::INVALID_FILE);
                }
                if (Error err = tabs::open(filepath); err != Error::OK) {
                    return err;
                }
            }
        }
        if (gui) {
            return -1;
        }
        return 0;
    }

    static void _print_help(void)
    {
        puts("Usage:\r\n"
             "  " APPNAME_BIN " [OPTIONS] [FILE ...]\r\n"
             "\r\n"
             "  A free and open-source cross-platform logic circuit "
             "simulator.\r\n"
             "\r\n"
             "Options:\r\n"
             "  -i, --interactive     Enters interactive mode.\r\n"
             "  -v, --version         Print version information.\r\n"
             "  -V, --verbose         Enable verbose logging.\r\n"
             "  -h, --help            Prints this section.\r\n"
             "\r\n"
             "Report bugs in the bug tracker at\r\n"
             "<https://github.com/umutsevdi/imcircuit/"
             "issues>\r\n"
             "or by email to <ask@umutsevdi.com>.\r\n");
    }

    static Replxx::completions_t completion_cb(
        std::string const& text, int& len)
    {
        std::vector<Replxx::Completion> filtered;

        for (const auto& token : root) {
            if (token.name.find(text) == 0) {
                // Continue matching
                if (token.type != Type::NONE) {
                    filtered.emplace_back(
                        token.name + " ", Replxx::Color::DEFAULT);
                } else {
                    filtered.emplace_back(token.name, Replxx::Color::DEFAULT);
                }
            }
            if (token.name == text) {
                break;
            }
        }
        if (!filtered.empty()) {
            len = text.size();
        }
        return filtered;
    }

    static replxx::Replxx::hints_t hint_cb(
        std::string const& text, int& len, Replxx::Color& color)
    {
        color = Replxx::Color::YELLOW;
        std::vector<std::string> filtered;
        for (const auto& token : root) {
            if (token.name.find(text) == 0) {
                filtered.emplace_back(token.msg.data());
            }
        }
        if (!filtered.empty()) {
            len = text.size();
        }
        return filtered;
    }

    int run(void)
    {
        Replxx replxx {};
        std::string history_file = (fs::CACHE / "historyfile").string();
        replxx.history_load(history_file);
        replxx.set_beep_on_ambiguous_completion(true);
        replxx.set_completion_callback(completion_cb);
        replxx.set_hint_callback(hint_cb);
        std::string line;
        while (keep_shell()) {
            auto scene = tabs::active();
            if (scene == nullptr) {
                line = replxx.input("(empty) # ");
            } else {
                line = replxx.input("(" + std::string { scene->name().data() }
                    + (tabs::is_saved() ? ") # " : "*) # "));
            }
            if (line.empty()) {
                continue;
            }
            bool matched = false;
            for (Command& cmd : root) {
                std::string arg;
                if (cmd.is_matching(line, arg)) {
                    Error err = cmd.cmd(scene, arg);
                    if (err == Error::OK) {
                        replxx.history_add(line);
                    } else if (err == Error::INVALID_ARGUMENT) {
                        L_ERROR("Expected argument type of %s, found %s",
                            to_str<Type>(cmd.type), arg.c_str());
                    }
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                L_ERROR("Unrecognized command <%s>.", line.c_str());
            }
        }
        replxx.history_save(history_file);
        return 0;
    }

} // namespace cli
} // namespace ic
