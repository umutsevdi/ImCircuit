#include <replxx.hxx>

#include "cli.h"
#include "common.h"
#include "core.h"

namespace lcs {
template <> const char* to_str<cli::Type>(cli::Type v)
{
    switch (v) {
    case lcs::cli::NONE: return "<none>";
    case lcs::cli::STR: return "<string>";
    case lcs::cli::INT: return "<int>"; ;
    case lcs::cli::BOOL: return "<bool>";
    case lcs::cli::NODE: return "<node>";
    case lcs::cli::NODE_INT_INT: return "<node int int>";
    case lcs::cli::NODE_INT_NODE_INT: return "<node int node int>";
    }
}
namespace cli {
    using namespace replxx;
    static void print_help(void);

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
                print_help();
                exit(0);
            } else if (arg == "-v" || arg == "--version") {
                printf(APPOS "." APPBUILD "." APPVERSION "\r\n");
                exit(0);
            } else if (arg == "-V" || arg == "--verbose") {
                fs::is_verbose = true;
            } else {
                std::filesystem::path filepath { arg };
                if (!(filepath.has_extension()
                        && filepath.extension() == ".lcs")) {
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

    static void print_help(void)
    {
        printf("Usage:\r\n"
               "  " APPNAME_BIN " [OPTIONS] [FILE ...]\r\n"
               "\r\n"
               "  A free and open-source cross-platform Logic Circuit "
               "Simulator.\r\n"
               "\r\n"
               "Options:\r\n"
               "  -i, --interactive     Enters interactive mode.\r\n"
               "  -v, --version         Print version information.\r\n"
               "  -V, --verbose         Enable verbose logging.\r\n"
               "  -h, --help            Prints this section.\r\n"
               "\r\n"
               "Report bugs in the bug tracker at\r\n"
               "<https://github.com/umutsevdi/logic-circuit-simulator-2/"
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

    bool keep = true;
    int run(void)
    {
        Replxx replxx {};
        replxx.history_load(fs::CACHE / "historyfile");
        replxx.set_beep_on_ambiguous_completion(true);

        replxx.set_completion_callback(completion_cb);
        replxx.set_hint_callback(hint_cb);

        std::string line;
        while (keep) {
            auto scene = tabs::active();
            if (scene == nullptr) {
                line = replxx.input("(empty) # ");
            } else {
                line = replxx.input("(" + std::string { scene->name().data() }
                    + (tabs::is_saved() ? ") # " : "*) # "));
            }
            if (line.empty()) {
                continue;
            } else if (line == "exit") {
                keep = false;
                break;
            }
            bool found = false;
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
                    found = true;
                    break;
                }
            }
            if (!found) {
                L_ERROR("Unrecognized command <%s>.", line.c_str());
            }
        }
        replxx.history_save(fs::CACHE / "historyfile");
        return 0;
    }

} // namespace cli
} // namespace lcs
