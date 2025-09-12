#include <csignal>
#include "cli.h"
#include "common.h"
using namespace lcs;

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
    context.setOption("--reporters", "lcs");
    context.applyCommandLine(argc, argv);
    return context.run();
}
#else
#ifdef IMC_GUI
namespace lcs::ui {
extern int run(void);
}
#endif
constexpr int START_UI = -1;
static int _start(int argc, char* argv[])
{
    int code = cli::parse_args(argc, argv);
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
    return _start(__argc, __argv);
}
#else
int main(int argc, char* argv[]) { return _start(argc, argv); }
#endif
#endif
