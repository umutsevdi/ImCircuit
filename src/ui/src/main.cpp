#include <chrono>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>
#include <nfd.h>
#include "core.h"
#include "ui.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900)                                    \
    && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

namespace ic::ui {
std::vector<Window*> WINDOW_LIST;

static void glcb(int error, const char* description)
{
    L_ERROR("GLFW Error %d: %s\n", error, description);
}
static double get_timediff(void)
{
    using namespace std::chrono;
    static const auto start  = steady_clock::now();
    auto now                 = steady_clock::now();
    duration<double> elapsed = now - start;
    return elapsed.count();
}

/** Attempts to reduce resource usage if no animation is being played. */
static bool save_power(void)
{
    if (ic::ui::has_notifications()) {
        return false;
    }
    constexpr float FPS_IDLE   = 10.f; // FPS while saving
    constexpr double TOLERANCE = 0.9;
    double before_wait         = get_timediff();
    double wait_timeout        = 1. / (double)FPS_IDLE;

    glfwWaitEventsTimeout(wait_timeout);

    double after_wait    = get_timediff();
    double duration      = (after_wait - before_wait);
    double expected_idle = 1. / FPS_IDLE;
    return duration > expected_idle * TOLERANCE;
}

int run()
{
    glfwSetErrorCallback(glcb);
    if (!glfwInit()) {
        return 1;
    }
// GL GLSL Versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    Configuration& cfg = Configuration::load();
    GLFWwindow* window = glfwCreateWindow(
        cfg.startup_win_x, cfg.startup_win_y, APPNAME, nullptr, nullptr);
    if (window == nullptr) {
        return 1;
    }
    glfwMakeContextCurrent(window);
    if (cfg.start_fullscreen) {
        glfwMaximizeWindow(window);
    }
    glfwSwapInterval(1);
    IMGUI_CHECKVERSION();
    ui::bind_config(ImGui::CreateContext());
    ImGuiIO& imio = ImGui::GetIO();
    (void)imio;
    std::string _inistr = (fs::CONFIG / "default.ini").string();
    imio.IniFilename    = _inistr.c_str();
    imio.ConfigFlags
        |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
    imio.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    NFD_Init();
    ImNodes::CreateContext();
    set_style(imio, true);
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    WINDOW_LIST   = INIT_WINDOW_LIST();
    bool switched = false;
    while (!glfwWindowShouldClose(window)) {
        bool is_enabled = save_power();
        if (is_enabled && !switched) {
            L_DEBUG("Started power saving.");
            switched = true;
        } else if (switched && !is_enabled) {
            L_DEBUG("Stopped power saving.");
            switched = false;
        }
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport();
        MenuBar();
        Ref<Scene> scene = tabs::active();
        bool is_changed  = tabs::is_changed();
        if (scene != nullptr) {
            scene->run(imio.DeltaTime);
        }
        for (auto& win : WINDOW_LIST) {
            if (win->is_active) {
                win->loop(scene, is_changed);
            }
        }
#ifndef NDEBUG
        ImGui::ShowDemoWindow(nullptr);
#endif
        show_notifications();
        ImGui::Render();
        if (!Configuration::get().is_applied) {
            set_style(imio);
        }
        ImVec4 clear_color = get_active_style().bg;

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w,
            clear_color.y * clear_color.w, clear_color.z * clear_color.w,
            clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }
    if (tabs::active() != nullptr) {
        if (tabs::is_saved()) {
            tabs::close();
        }
        //      FIXME
        //      else {
        //          int option = tinyfd_messageBox("Close Scene",
        //              "You have unsaved changes. Would you like to save your
        //              changes " "before closing?", "yesno", "question", 0);
        //          if (!option || save_as_flow("Save scene")) {
        //              tabs::scene_close();
        //          }
        //      }
    }
    ImNodes::DestroyContext();
    NFD_Quit();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

} // namespace ic::ui
