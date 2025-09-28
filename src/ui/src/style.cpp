#include <IconsLucide.h>
#include <cstdlib>
#include <imgui.h>
#include <imnodes.h>
#include <json/reader.h>
#include <json/value.h>
#include "common.h"
#include "components.h"
#include "ui.h"

#define DL(_a, _b) (t.is_dark ? (_a) : (_b))

namespace ic::ui {
static std::map<std::string, Theme> themes;
static std::vector<const char*> names_light {};
size_t selected_light = 0;
static std::vector<const char*> names_dark {};
size_t selected_dark                   = 0;
static ImFont* _FONT[FontType::FONT_S] = {};

static Error _read_theme(Theme& theme, const Json::Value& v);
static void _init_fonts(ImGuiIO& io);
static void _init_default_themes(void);
static void _init_themes(Configuration& cfg);
static void _set_colors(
    ImVec4 clr_gui[ImGuiCol_COUNT], ImU32 clr_node[ImNodesCol_COUNT]);

ImFont* get_font(FontType type) { return _FONT[type]; }

void set_style(ImGuiIO& io, bool init)
{
    Configuration& cfg      = Configuration::get();
    cfg.is_applied          = true;
    ImGuiStyle& style       = ImGui::GetStyle();
    ImNodesStyle& nodestyle = ImNodes::GetStyle();
    if (init) {
        _init_fonts(io);
        _init_themes(cfg);
    }
    _set_colors(style.Colors, nodestyle.Colors);

    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.Alpha                    = 1.0f;
    style.FrameRounding            = cfg.rounded_corners;
    style.WindowRounding           = cfg.rounded_corners;
    style.ChildRounding            = cfg.rounded_corners;
    style.GrabRounding             = cfg.rounded_corners;
    style.PopupRounding            = cfg.rounded_corners;
    style.ScrollbarRounding        = cfg.rounded_corners;
    style.TabRounding              = cfg.rounded_corners;

    style.WindowTitleAlign = ImVec2(0.5, 0.5);
    style.WindowPadding    = ImVec2(10, 15);
    style.FramePadding     = ImVec2(5, 5);
    style.ItemSpacing      = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(4, 2);
    style.IndentSpacing    = 25.0f;
    style.ScrollbarSize    = 10.0f;
    style.GrabMinSize      = 5.0f;

    style.ChildBorderSize         = 1.f;
    style.FrameBorderSize         = 0.f;
    style.PopupBorderSize         = 1.f;
    style.TabBarBorderSize        = 1.f;
    style.WindowBorderSize        = 1.f;
    style.SeparatorTextBorderSize = 0.f;

    style.FontSizeBase  = FONT_NORMAL;
    style.FontScaleMain = cfg.scale / 100.f;
    style.ScaleAllSizes(cfg.scale / 100.f);
    if (init) {
        L_DEBUG("Styling is completed.");
    }
}

const char* get_style(size_t idx, bool is_dark)
{
    if (is_dark) {
        return names_dark.at(idx);
    }
    return names_light.at(idx);
}

const Theme& get_theme(const std::string& s)
{
    if (auto t = themes.find(s); t != themes.end()) {
        return t->second;
    }
    ic_assert(!names_dark.empty() && !names_light.empty());
    return themes.find(names_dark[0])->second;
}

const std::vector<const char*>& get_available_styles(bool is_dark)
{
    return is_dark ? names_dark : names_light;
}

const Theme& get_active_style(void)
{
    Configuration& config = Configuration::get();
    return get_theme(config.preference == Configuration::ALWAYS_LIGHT
            ? config.light_theme
            : config.dark_theme);
}

static void _init_fonts(ImGuiIO& io)
{
    L_DEBUG("Load fonts.");
    // Load the fonts twice the size and scale them back to have clear
    // visuals.
    io.Fonts->AddFontDefault();
    auto dir = fs::APPDATA / "font" / "MPLUS1p";

    std::string font = (dir / "MPLUS1p-Regular.ttf").string();
    _FONT[REGULAR]   = io.Fonts->AddFontFromFileTTF(font.c_str());
    font             = (dir / "MPLUS1p-Bold.ttf").string();
    _FONT[BOLD]      = io.Fonts->AddFontFromFileTTF(font.c_str());
    font             = (dir / "MPLUS1p-Light.ttf").string();
    _FONT[LIGHT]     = io.Fonts->AddFontFromFileTTF(font.c_str());
    font           = (fs::APPDATA / "font" / "Lucide" / "Lucide.ttf").string();
    _FONT[ICON]    = io.Fonts->AddFontFromFileTTF(font.c_str());
    io.FontDefault = _FONT[REGULAR];
}

#define CLRU32(...) ImGui::GetColorU32(__VA_ARGS__)

static void _set_colors(ImVec4 ig[ImGuiCol_COUNT], ImU32 in[ImNodesCol_COUNT])
{
    const Theme& t = get_active_style();

    ig[ImGuiCol_WindowBg]       = t.bg;
    ig[ImGuiCol_Text]           = t.fg;
    ig[ImGuiCol_TextDisabled]   = v4mul(t.fg, 0.7f);
    ig[ImGuiCol_TextSelectedBg] = v4mul(t.blue, 0.8f);
    ig[ImGuiCol_TextLink]       = v4mul(t.blue, 1.2f);

    // Headers
    ig[ImGuiCol_MenuBarBg]     = v4mul(t.bg, DL(0.5f, 0.8f));
    ig[ImGuiCol_Header]        = v4mul(DL(t.black, t.white), DL(0.5f, 0.8f));
    ig[ImGuiCol_HeaderHovered] = v4mul(DL(t.black, t.white), DL(0.7f, 1.f));
    ig[ImGuiCol_HeaderActive]  = v4mul(DL(t.black, t.white), DL(0.75f, 1.05f));

    // Buttons
    ig[ImGuiCol_Button]        = v4mul(DL(t.blue, t.blue), DL(0.5f, 0.9f));
    ig[ImGuiCol_ButtonHovered] = v4mul(t.blue, DL(0.8f, 1.0f));
    ig[ImGuiCol_ButtonActive]  = v4mul(t.blue, DL(0.8f, 1.1f));
    ig[ImGuiCol_CheckMark]     = t.yellow;

    ig[ImGuiCol_DockingPreview] = v4mul(t.blue, 1.f, 0.5f);
    ig[ImGuiCol_DockingEmptyBg] = v4mul(t.blue, 1.f, 0.5f);

    // Frame BG
    ig[ImGuiCol_FrameBg]          = v4mul(t.bg, DL(0.5f, 0.8f));
    ig[ImGuiCol_FrameBgHovered]   = v4mul(t.bg, DL(1.7f, 1.2f));
    ig[ImGuiCol_FrameBgActive]    = v4mul(t.bg, DL(1.7f, 1.2f));
    ig[ImGuiCol_ChildBg]          = v4mul(t.bg, DL(0.75f, 0.9f));
    ig[ImGuiCol_PopupBg]          = v4mul(t.bg, DL(0.75f, 0.9f));
    ig[ImGuiCol_ModalWindowDimBg] = v4mul(DL(t.black, t.white), 0.5f, 0.5f);

    // Tabs
    ig[ImGuiCol_Tab]                = v4mul(DL(t.black, t.white), 0.8f);
    ig[ImGuiCol_TabHovered]         = v4mul(DL(t.black, t.white), 1.f);
    ig[ImGuiCol_TabActive]          = v4mul(DL(t.black, t.white), 0.9f);
    ig[ImGuiCol_TabUnfocused]       = v4mul(DL(t.black, t.white), 0.5f);
    ig[ImGuiCol_TabUnfocusedActive] = v4mul(DL(t.black, t.white), 0.7f);

    // Title
    ig[ImGuiCol_TitleBg]          = v4mul(DL(t.black, t.white), 0.8f);
    ig[ImGuiCol_TitleBgActive]    = v4mul(DL(t.black, t.white), 0.9f);
    ig[ImGuiCol_TitleBgCollapsed] = t.bg;

    ig[ImGuiCol_Border]            = v4mul(DL(t.black, t.white), 0.9f);
    ig[ImGuiCol_ResizeGrip]        = v4mul(DL(t.black, t.white), 0.9f);
    ig[ImGuiCol_ResizeGripHovered] = t.fg;
    ig[ImGuiCol_ResizeGripActive]  = t.fg;
    ig[ImGuiCol_Separator]         = v4mul(DL(t.black, t.white), 0.9f);
    ig[ImGuiCol_SeparatorHovered]  = t.fg;
    ig[ImGuiCol_SeparatorActive]   = t.fg;
    ig[ImGuiCol_TableBorderStrong] = v4mul(DL(t.black, t.white), 0.9f);
    ig[ImGuiCol_TableBorderLight]  = v4mul(DL(t.black, t.white), 0.9f);

    ig[ImGuiCol_ScrollbarBg]   = v4mul(DL(t.black, t.white), DL(0.5f, 0.8f));
    ig[ImGuiCol_ScrollbarGrab] = v4mul(DL(t.black, t.white), 0.9f);
    ig[ImGuiCol_ScrollbarGrabHovered]
        = v4mul(DL(t.black, t.white), DL(0.7f, 1.f));
    ig[ImGuiCol_ScrollbarGrabActive]
        = v4mul(DL(t.black, t.white), DL(0.75f, 1.05f));

    ig[ImGuiCol_TableHeaderBg] = v4mul(t.yellow, DL(0.8f, 1.f));
    ig[ImGuiCol_TableRowBg]    = t.bg;
    ig[ImGuiCol_TableRowBgAlt] = v4mul(t.bg, 0.8f);

    ig[ImGuiCol_PlotLines]            = v4mul(t.green, 0.9f);
    ig[ImGuiCol_PlotLinesHovered]     = v4mul(t.green, 1.0f);
    ig[ImGuiCol_PlotHistogram]        = v4mul(t.green, 0.9f);
    ig[ImGuiCol_PlotHistogramHovered] = v4mul(t.green, 1.0f);
    in[ImNodesCol_GridBackground]     = CLRU32(v4mul(t.bg, DL(1.2f, 0.8f)));
    in[ImNodesCol_GridLine]           = CLRU32(v4mul(t.fg, 0.5f));
    //    clr_node[ImNodesCol_GridLinePrimary] = CLRU32(ImGuiCol_FrameBgActive);
    in[ImNodesCol_NodeBackground]         = CLRU32(ImGuiCol_WindowBg);
    in[ImNodesCol_NodeBackgroundHovered]  = CLRU32(v4mul(t.bg, 1.3f));
    in[ImNodesCol_NodeBackgroundSelected] = CLRU32(v4mul(t.bg, 1.5f));

    in[ImNodesCol_NodeOutline] = CLRU32(v4mul(t.fg, 0.5f));

    in[ImNodesCol_TitleBar]           = CLRU32(ImGuiCol_Tab);
    in[ImNodesCol_TitleBarHovered]    = CLRU32(ImGuiCol_TabHovered);
    in[ImNodesCol_TitleBarSelected]   = CLRU32(ImGuiCol_TabActive);
    in[ImNodesCol_Link]               = CLRU32(ImGuiCol_Button);
    in[ImNodesCol_LinkHovered]        = CLRU32(ImGuiCol_ButtonHovered);
    in[ImNodesCol_LinkSelected]       = CLRU32(ImGuiCol_ButtonActive);
    in[ImNodesCol_Pin]                = CLRU32(v4mul(t.yellow, 1.0f));
    in[ImNodesCol_PinHovered]         = CLRU32(v4mul(t.yellow, 1.2f));
    in[ImNodesCol_BoxSelector]        = CLRU32(ImGuiCol_DockingPreview);
    in[ImNodesCol_BoxSelectorOutline] = CLRU32(ImGuiCol_DockingEmptyBg);

    in[ImNodesCol_MiniMapBackground]        = CLRU32(ImGuiCol_WindowBg);
    in[ImNodesCol_MiniMapBackgroundHovered] = CLRU32(ImGuiCol_WindowBg);
    in[ImNodesCol_MiniMapOutline]           = CLRU32(ImGuiCol_FrameBg);
    in[ImNodesCol_MiniMapOutlineHovered]    = CLRU32(ImGuiCol_FrameBgHovered);

    in[ImNodesCol_MiniMapNodeBackground] = CLRU32(ImGuiCol_WindowBg);
    in[ImNodesCol_MiniMapNodeBackgroundHovered]
        = CLRU32(v4mul(DL(t.black, t.white), 1.1f));
    in[ImNodesCol_MiniMapNodeBackgroundSelected]
        = CLRU32(v4mul(DL(t.black, t.white), 1.1f));
}

static void _init_themes(Configuration& cfg)
{
    L_DEBUG("Load themes.");
    _init_default_themes();
    Json::Value v {};
    Json::Reader r;
    std::string data;
    if (fs::read(fs::CONFIG / "themes.json", data)) {
        ic_assert(r.parse(data, v));
        ic_assert(v.isArray());
        for (const auto& element : v) {
            Theme s;
            if (_read_theme(s, element) != Error::OK) {
                L_WARN("Invalid theme: %s", v.toStyledString().c_str());
            }
            themes.emplace(s.name, s);
        }
    }
    names_light.reserve(themes.size());
    names_dark.reserve(themes.size());
    for (const auto& s : themes) {
        if (s.second.is_dark) {
            names_dark.push_back(s.first.c_str());
        } else {
            names_light.push_back(s.first.c_str());
        }
    }
    if (themes.find(cfg.light_theme) == themes.end()) {
        L_ERROR("Light theme preference %s were not found. Falling back to %s",
            cfg.light_theme.c_str(), names_light[0]);
        cfg.light_theme = names_light[0];
    }
    if (themes.find(cfg.dark_theme) == themes.end()) {
        L_ERROR("Dark theme preference %s were not found. Falling back to %s",
            cfg.dark_theme.c_str(), names_dark[0]);
        cfg.dark_theme = names_dark[0];
    }
}

inline ImVec4 to_imvec4(uint64_t clr)
{
    return ImVec4(((clr >> 16) & 0xFF) / 255.0f, ((clr >> 8) & 0xFF) / 255.0f,
        (clr & 0xFF) / 255.0f, 1.0f);
}

inline ImVec4 to_imvec4(const std::string& hex)
{
    ic_assert(hex.size() == 7 || hex[0] == '#');
    return to_imvec4(std::strtoul(hex.c_str() + 1, nullptr, 16));
}

static Error _read_theme(Theme& theme, const Json::Value& v)
{
    if (!(v["name"].isString() && v["is_dark"].isBool() && v["bg"].isString()
            && v["fg"].isString() && v["black"].isString()
            && v["red"].isString() && v["green"].isString()
            && v["yellow"].isString() && v["blue"].isString()
            && v["magenta"].isString() && v["cyan"].isString()
            && v["white"].isString() && v["gray"].isString())) {
        return ERROR(Error::INVALID_JSON);
    }
    theme.name    = v["name"].asString();
    theme.is_dark = v["is_dark"].asBool();
    theme.bg      = to_imvec4(v["bg"].asString());
    theme.fg      = to_imvec4(v["fg"].asString());
    theme.black   = to_imvec4(v["black"].asString());
    theme.red     = to_imvec4(v["red"].asString());
    theme.green   = to_imvec4(v["green"].asString());
    theme.yellow  = to_imvec4(v["yellow"].asString());
    theme.blue    = to_imvec4(v["blue"].asString());
    theme.magenta = to_imvec4(v["magenta"].asString());
    theme.white   = to_imvec4(v["white"].asString());
    theme.gray    = to_imvec4(v["gray"].asString());
    return OK;
}

static void _init_default_themes(void)
{
    Theme light;
    Theme dark;
    // Colors (Ayu Light)
    // Default colors - taken from ayu-colors
    light.name    = "Default (Light)";
    light.is_dark = false;
    light.bg      = to_imvec4(0xFCFCFC);
    light.fg      = to_imvec4(0x5C6166);
    light.black   = to_imvec4(0x010101);
    light.red     = to_imvec4(0xe7666a);
    light.green   = to_imvec4(0x80ab24);
    light.yellow  = to_imvec4(0xeba54d);
    light.blue    = to_imvec4(0x4196df);
    light.magenta = to_imvec4(0x9870c3);
    light.white   = to_imvec4(0xc1c1c1);
    light.gray    = to_imvec4(0x343434);
    themes.emplace("Default (Light)", std::move(light));

    // name: SeaShells
    // https//raw.githubusercontent.com/mbadolato/iTerm2-Color-Schemes/master/schemes/SeaShells.itermcolors
    dark.name    = "Default (Dark)";
    dark.is_dark = true;
    dark.bg      = to_imvec4(0x061923);
    dark.fg      = to_imvec4(0xe5c49e);
    dark.black   = to_imvec4(0x1d485f);
    dark.red     = to_imvec4(0xdb662d);
    dark.green   = to_imvec4(0x008eab);
    dark.yellow  = to_imvec4(0xfeaf3c);
    dark.blue    = to_imvec4(0x255a62);
    dark.magenta = to_imvec4(0x77dbf4);
    dark.white   = to_imvec4(0xe5c49e);
    dark.gray    = to_imvec4(0x545d65);
    themes.emplace("Default (Dark)", std::move(dark));
}

} // namespace ic::ui
