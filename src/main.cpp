#include <cstdlib>
#ifndef __TESTING__
#define __TESTING__ 0
#endif

#if __TESTING__
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#endif

#include "common.h"
#include "port.h"

namespace lcs::ui {
extern int init(void);
}
using namespace lcs;
static void _cleanup(void)
{
    fs::close();
    net::close();
}

#if defined(_WIN32) && NDEBUG && __TESTING__ == 0
#include <windows.h>
int WINAPI WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    fs::init(__TESTING__);
    net::init(__TESTING__);
    std::atexit(_cleanup);
    return ui::init();
}
#else
int main(int argc, char* argv[])
{
    fs::init(__TESTING__);
    net::init(__TESTING__);
    std::atexit(_cleanup);
#if __TESTING__
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    return context.run();
#else
    return ui::init();
#endif
}
#endif
