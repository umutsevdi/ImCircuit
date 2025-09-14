#pragma once
/*******************************************************************************
 * \file
 * File: ui.h
 * Created: 24/06/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/imcircuit
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include <imgui.h>
#include "common.h"
#include "core.h"

namespace ic::ui {
void show_notifications(void);
bool has_notifications(void);

inline ImVec4 v4mul(ImVec4 vec, float pct)
{
    return ImVec4(vec.x * pct, vec.y * pct, vec.z * pct, vec.w);
}

inline ImVec4 v4mul(ImVec4 vec, float pct, float alpha)
{
    return ImVec4(vec.x * pct, vec.y * pct, vec.z * pct, vec.w * alpha);
}

void MenuBar(void);
void Profile(std::string& name, bool switched);

struct Theme {
    std::string name = "";
    bool is_dark     = false;
    ImVec4 bg;
    ImVec4 fg;
    ImVec4 black;
    ImVec4 red;
    ImVec4 green;
    ImVec4 yellow;
    ImVec4 blue;
    ImVec4 magenta;
    ImVec4 cyan;
    ImVec4 white;
    ImVec4 black_bright;
    ImVec4 red_bright;
    ImVec4 green_bright;
    ImVec4 yellow_bright;
    ImVec4 blue_bright;
    ImVec4 magenta_bright;
    ImVec4 cyan_bright;
    ImVec4 white_bright;
};
const Theme& get_theme(const std::string& s);
const Theme& get_active_style(void);
const std::vector<const char*>& get_available_styles(bool is_dark);
void set_style(ImGuiIO& io, bool init = false);

/** Embed and sync the UserData struct to ImGUIs's configuration file. */
void bind_config(ImGuiContext*);

class Configuration {
public:
    enum Preference {
        FOLLOW_OS,
        ALWAYS_LIGHT,
        ALWAYS_DARK,
    };
    Configuration()  = default;
    ~Configuration() = default;

    bool is_saved           = true;
    bool is_applied         = true;
    std::string light_theme = "Default (Light)";
    std::string dark_theme  = "Default (Dark)";
    Preference preference   = FOLLOW_OS;
    int rounded_corners     = 0;
    int scale               = 100;
    std::string api_proxy   = API_ENDPOINT;
    std::string language    = "en";
    int startup_win_x       = 1980;
    int startup_win_y       = 1080;
    bool start_fullscreen   = true;
    std::string login;

    static Configuration& get(void);
    static void set(Configuration&);
    static void save(void);
    static Configuration& load(void);
};

namespace popup {
    void LoginWindow(bool& df_show);
} // namespace popup

namespace dialog {
    Error open_file(void);
    Error save_file_as(void);
} // namespace dialog

/******************************************************************************
                                Window Manager
*****************************************************************************/

/**
 * Every ImGUI Window implements the following structure. After declaring the
 * struct use REGISTER_WINDOW("Window Name", WindowImpl) to register the window
 * to the registry.
 *
 * Then add the window name to the following list
 *
 */
struct Window {
    Window(const char* _name, ImGuiWindowFlags _flags = 0)
        : basename { _name }
        , _flags { _flags }
    {
        L_DEBUG("Window %s is ready.", _name);
    };
    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&)                 = delete;
    Window& operator=(Window&&)      = delete;
    virtual ~Window()                = default;

    void loop(Ref<Scene>, bool);

    const char* basename;
    bool is_active = true;

protected:
    /**
     * Returns a consistent identifier for given window so that
     * the window position can be retained even after the language change.
     */
    const char* title(void)
    {
        std::snprintf(_title, 120, "%s###%s", _(basename), basename);
        return _title;
    }

    /**
     * Window::show will be executed in every frame.
     * @param scene provides a pointer to the scene if a scene is active.
     * @param switched if the tab has just been switched.
     */
    virtual void show(Ref<Scene> scene, bool switched) = 0;
    virtual const char* tooltip(void)                  = 0;

    const char* _shortcut = nullptr;
    ImGuiWindowFlags _flags;
    char _title[120];
};

/* Returns a pointer to the registered window name */
#define WINDOWNAME(ID) _window_by_id<_get_id(ID)>()
/**
 * Declare your windows here. It must match with the string in the
 * REGISTER_WINDOW macro.
 */
#define INIT_WINDOW_LIST()                                                     \
    {                                                                          \
        WINDOWNAME("Console"),                                                 \
        WINDOWNAME("Inspector"),                                               \
        WINDOWNAME("Editor"),                                                  \
        WINDOWNAME("Palette"),                                                 \
        WINDOWNAME("Property Editor"),                                         \
        WINDOWNAME("Scene Info"),                                              \
    };
/* Registers a window CLASS to the provided ID. Automatically implements the
 * constructor. Every subsequent call to this method returns the same object. */
#define REGISTER_WINDOW(ID, CLASS, ...)                                        \
    CLASS::CLASS()                                                             \
        : Window(ID) {                                                         \
            __VA_ARGS__                                                        \
        };                                                                     \
    template <> Window* WINDOWNAME(ID)                                         \
    {                                                                          \
        static CLASS instance;                                                 \
        return &instance;                                                      \
    }
extern std::vector<Window*> WINDOW_LIST;

/** Hash function to convert window names to unique identifiers at compile time.
 */
constexpr std::uint64_t _fnv1a(
    const char* str, std::size_t n, std::uint64_t h = 14695981039346656037ULL)
{
    return n == 0
        ? h
        : _fnv1a(str + 1, n - 1,
              (h ^ static_cast<std::uint64_t>(str[0])) * 1099511628211ULL);
}

/** Generates a window identifier using the provided string at compile time. */
template <std::size_t N> constexpr std::uint64_t _get_id(const char (&s)[N])
{
    return _fnv1a(s, N - 1);
}

/* Returns a pointer to a window with given identifier. Every subsequent call
 * to this method returns the same object. */
template <uint64_t T> Window* _window_by_id();

} // namespace ic::ui
