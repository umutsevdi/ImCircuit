#include <csignal>
#include "cli.h"
#include "common.h"
using namespace ic;

#if IMC_TEST
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
int main(int argc, char* argv[])
{
    fs::init(IMC_TEST);
    net::init(IMC_TEST);
    std::atexit([]() {
        fs::close();
        net::close();
    });
    doctest::Context context;
    context.setOption("--reporters", "ic");
    context.applyCommandLine(argc, argv);
    return context.run();
}
#else
#ifdef IMC_GUI
namespace ic::ui {
extern int run(void);
}
#endif
static int entrypoint(int argc, char* argv[])
{
    constexpr int START_UI = -1;
    int code = cli::parse_args(argc, argv);
    if (code > 0) {
        return code;
    }
    fs::init();
    net::init();
    std::atexit([]() {
        fs::close();
        net::close();
    });
    if (code == START_UI) {
#ifdef IMC_GUI
        return ui::run();
#else
        code = ERROR(Error::GUI_NOT_SUPPORTED);
#endif
    }
    if (code == 0) {
        return cli::run();
    }
    return code;
}
#if defined(_WIN32) && defined(IMC_GUI) == 1 && NDEBUG
#include <windows.h>
int WINAPI WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return entrypoint(__argc, __argv);
}
#else
int main(int argc, char* argv[]) { return entrypoint(argc, argv); }
#endif
#endif
