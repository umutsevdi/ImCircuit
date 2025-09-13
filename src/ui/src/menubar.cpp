#include <IconsLucide.h>
#include <imgui.h>
#include <imnodes.h>
#include "common.h"
#include "components.h"
#include "ui.h"

namespace ic::ui {

Configuration cfg {};
static bool _show_new       = false;
static bool _show_close     = false;
static bool _show_about     = false;
static bool _show_pref      = false;
static bool _show_changelog = false;
static void tab_window(void);
static void _popup_new(void);
static void _popup_about(void);
static void _popup_close(void);
static void _popup_pref(void);
static void _changelog_popup(void);
static ImVec2 line_size(0, 0);
bool df_show = false;
void MenuBar(void)
{
    ImGui::PushStyleColor(
        ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
    ImGui::PushStyleColor(
        ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));

    ImGui::PushFont(nullptr, 0.f);
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu(_("File"))) {
            if (IconButton(ICON_LC_PLUS, _("New"))) {
                _show_new = true;
            }
            if (IconButton(ICON_LC_FOLDER_OPEN, _("Open"))) {
                dialog::open_file();
            }
            ImGui::BeginDisabled(tabs::active() == nullptr);
            if (IconButton(ICON_LC_SAVE, _("Save"))) {
                if (tabs::save() == Error::NO_SAVE_PATH_DEFINED) {
                    dialog::save_file_as();
                };
            }

            if (IconButton(ICON_LC_SAVE_ALL, _("Save As"))) {
                dialog::save_file_as();
            }
            ImGui::EndDisabled();
            if (IconButton(ICON_LC_SETTINGS_2, _("Preferences"))) {
                _show_pref = true;
            }
            if (IconButton(ICON_LC_X, _("Close"))) {
                _popup_close();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(_("Edit"))) {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(_("View"))) {
            for (auto w : WINDOW_LIST) {
                ImGui::Checkbox(_(w->basename), &w->is_active);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(_("Help"))) {
            if (IconButton(ICON_LC_GIT_MERGE, _("Changelog"))) {
                _show_changelog = true;
            }
            if (IconButton(ICON_LC_COPYRIGHT, _("About"))) {
                _show_about = true;
            }
            ImGui::EndMenu();
        }
        ImGui::Separator();
        ImGui::SetCursorPosY(ImGui::GetStyle().ItemSpacing.y
            - ImGui::GetStyle().ItemInnerSpacing.y);

        tab_window();
        //        ImVec2 pos        = ImGui::GetWindowContentRegionMax();
        //      ImVec2 login_text = ImGui::CalcTextSize("Login");

        //      ImGui::PushStyleColor(
        //          ImGuiCol_Button,
        //          ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
        //      ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        //          ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
        //      ImGui::PushStyleColor(ImGuiCol_ButtonActive,
        //          ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        //      if (auth::is_logged_in()) {
        //          const ImageHandle* profile
        //              = get_texture(net::get_account().avatar_url);
        //          if (profile) {
        //              ImGui::PushStyleVar(
        //                  ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.1f, 0.1f));
        //              float h_image = (1.2 * login_text.y
        //                  + 2 * ImGui::GetStyle().ItemInnerSpacing.y);
        //              ImGui::SameLine(pos.x - ImGui::GetStyle().ItemSpacing.x
        //                  - profile->w / profile->h * h_image);
        //              if (ImGui::ImageButton(net::get_account().login.c_str(),
        //                      profile->gl_id,
        //                      ImVec2(profile->w / profile->h * h_image,
        //                      h_image), ImVec2(0, 0), ImVec2(1, 1))) {
        //                  ImGui::OpenPopup("Profile Settings");
        //              }
        //              if (ImGui::BeginPopup(
        //                      "Profile Settings", ImGuiWindowFlags_ChildMenu))
        //                      {
        //                  if (IconButton(ICON_LC_SETTINGS_2,
        //                  "Settings")) { } if
        //                  (IconButton(ICON_LC_LOG_OUT, "Log out")) {
        //                      net::get_flow().resolve();
        //                  }
        //                  ImGui::EndPopup();
        //              }
        //              ImGui::PopStyleVar();
        //          }
        //      } else {
        //          ImGui::SameLine(pos.x - 2 * login_text.x);
        //          ImGui::BeginDisabled(net::get_flow().get_state()
        //              == net::AuthenticationFlow::State::POLLING);
        //          ImGui::SetCursorPosY(ImGui::GetStyle().ItemSpacing.y
        //              + ImGui::GetStyle().ItemInnerSpacing.y);
        //          if (IconButton(ICON_LC_LOG_IN, "Login")) {
        //              L_INFO("Login button pressed");
        //              df_show = true;
        //          }
        //          ImGui::EndDisabled();
        //      }
        // ImGui::PopStyleColor();
        // ImGui::PopStyleColor();
        // ImGui::PopStyleColor();
        ImGui::EndMainMenuBar();
        //        popup::LoginWindow(df_show);
        //  ImGui::Separator();
    };
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    _popup_new();
    _popup_close();
    _popup_about();
    _popup_pref();
    _changelog_popup();
}

void tab_window(void)
{
    ImGui::BeginTabBar(_("Scene Tabs"), ImGuiTabBarFlags_FittingPolicyScroll);
    tabs::for_each([](std::string_view name, const std::filesystem::path& path,
                       bool is_saved, bool is_active) -> bool {
        bool result = false;
        bool keep   = true;

        std::string scene_name;
        if (!name.empty()) {
            scene_name = name.data();
        } else if (!path.empty()) {
            scene_name = path.string();
        } else {
            scene_name = _("Untitled Scene");
        }
        if (ImGui::BeginTabItem(scene_name.c_str(), &keep,
                (is_saved ? ImGuiTabItemFlags_None
                          : ImGuiTabItemFlags_UnsavedDocument))) {
            if (!is_active) {
                result = true;
                ImNodes::ClearLinkSelection();
                ImNodes::ClearNodeSelection();
            }
            ImGui::EndTabItem();
        }
        if (!keep) {
            _popup_close();
        }
        return result;
    });
    ImGui::PushStyleColor(
        ImGuiCol_Button, ImGui::GetStyleColorVec4(ImNodesCol_TitleBar));
    if (ImGui::TabItemButton("+")) {
        _show_new = true;
    }
    ImGui::PopStyleColor();
    ImGui::EndTabBar();
}

static void _popup_new(void)
{
    if (!_show_new) {
        return;
    }

    std::string title = std::string { _("New Scene") } + "###NewScene";
    ImGui::OpenPopup(title.c_str());
    if (ImGui::BeginPopupModal(title.c_str(), &_show_new,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
        static bool is_scene         = true;
        static char author[60]       = "local";
        static char name[128]        = { 0 };
        static char description[512] = { 0 };
        static size_t input_size     = 0;
        static size_t output_size    = 0;
        AnonTable("NewFlow", 0, 
            TablePair(Field(_("Scene Name")),
                ImGui::InputText("##SceneCreate_Name", name, 128,
                    ImGuiInputTextFlags_CharsNoBlank));

            TablePair(Field(_("Author")),
                ImGui::InputText("##SceneCreate_Author", author, 60,
                    ImGuiInputTextFlags_ReadOnly));

            TablePair(Field(_("Description")),
                ImGui::InputTextMultiline(
                    "##SceneCreate_Desc", description, 512));

            TablePair(
                Field(_("Type")),
                if (ImGui::RadioButton("##IsScene", is_scene)) {
                    is_scene = true;
                });
            ImGui::SameLine();
            IconText(ICON_LC_LAND_PLOT,FONT_NORMAL, _("Scene"));
            ImGui::SameLine();
            if (ImGui::RadioButton("##IsComponent", !is_scene)) {
                is_scene = false;
            }
            ImGui::SameLine();
            IconText(ICON_LC_PACKAGE,FONT_NORMAL, _("Component"));

            if (!is_scene) {
                ImGui::Separator();
                TablePair(Field(_("Input Size")),
                    ImGui::InputInt("##CompInputSize", (int*)&input_size));
                TablePair(Field(_("Output Size")),
                    ImGui::InputInt("##CompOutputSize", (int*)&output_size));
            }
            TablePair(
                if (ImGui::Button("Create")) {
                    Ref<Scene> scene = tabs::active(
                        tabs::create(name, author, description, 1));
                    if (!is_scene) {
                        scene->component_context.emplace(
                            &*scene, input_size, output_size);
                    }
                    _show_new = false;
                },
                if (ImGui::Button(_("Cancel"))) {
                    ImGui::CloseCurrentPopup();
                    _show_new = false;
                });
            );
        ImGui::EndPopup();
    }
}

static void _popup_close(void)
{
    if (!_show_close) {
        return;
    }
    if (tabs::active() == nullptr) {
        exit(0);
    }
    if (tabs::is_saved()) {
        if (!tabs::close()) {
            _show_close = false;
        }
    } else {
        std::string title = std::string { _("Close Scene") } + "###CloseScene";
        ImGui::OpenPopup(title.c_str());
        if (ImGui::BeginPopupModal(title.c_str(), &_show_close,
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::Text(_("You have unsaved changes. Would you like to save "
                          "your changes before closing?"));
            if (IconButton(ICON_LC_SAVE, _("Save & Close"))
                && dialog::save_file_as() == Error::OK) {
                if (!tabs::close()) {
                    _show_close = false;
                }
            }
            ImGui::SameLine();
            if (IconButton(ICON_LC_SAVE_OFF, _("Exit Without Saving"))) {
                if (!tabs::close()) {
                    _show_close = false;
                }
            }
        }
    }
}

static std::vector<uint8_t> license_info {};
static void _popup_about(void)
{
    if (license_info.empty()) {
        fs::read(fs::APPDATA / "LICENSE", license_info);
        license_info.push_back(0);
    }
    static const char* site = "https://umutsevdi.com";
    static const char* gh   = "https://github.com/umutsevdi";
    static const char* prj  = "https://github.com/umutsevdi/imcircuit";
    static const char* mail
        = "mailto:ask@umutsevdi.com?subject=\'About ImCircuit\'";
    if (!_show_about) {
        return;
    }
    if (line_size.x == 0) {
        line_size
            = ImGui::CalcTextSize("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                  "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    }
    std::string title = std::string { _("About") } + "###About";
    ImGui::OpenPopup(title.c_str());
    if (ImGui::BeginPopupModal(title.c_str(), &_show_about,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text(_("ImCircuit is a free and open-source cross-platform "
                      "desktop\napplication to simulate logic circuits."));
        ImGui::Text(_("Developer: "));
        ImGui::SameLine();
        if (ImGui::TextLink("Umut Sevdi")) {
            net::open_browser(site);
        }
        ImGui::Text(_("Source code is available at "));
        ImGui::SameLine();
        if (ImGui::TextLink("GitHub.")) {
            net::open_browser(prj);
        }
        ImGui::Text(_("Version: "));
        ImGui::SameLine();
        ImGui::TextUnformatted(
            APPPKG "." APPVERSION "." APPOS "." APPBUILD "\r\n");
        ImGui::BeginChild(
            "License", ImVec2(line_size.x, 450), ImGuiChildFlags_FrameStyle);
        ImGui::InputTextMultiline("##LicenseText", (char*)license_info.data(),
            license_info.size(), ImGui::GetContentRegionAvail(),
            ImGuiInputTextFlags_ReadOnly);
        ImGui::EndChild();
        ImGui::TextUnformatted(_("Contact"));
        ImGui::SameLine();
        if (IconButton(ICON_LC_MAIL, _("Email"))) {
            net::open_browser(mail);
        }
        ImGui::SameLine();
        if (IconButton(ICON_LC_LINK, _("Website"))) {
            net::open_browser(site);
        }
        ImGui::SameLine();
        if (IconButton(ICON_LC_GITHUB, "GitHub")) {
            net::open_browser(gh);
        }
        ImGui::EndPopup();
    }
}

static std::vector<uint8_t> changelog_info {};
static void _changelog_popup(void)
{
    if (changelog_info.empty()) {
        fs::read(fs::APPDATA / "CHANGELOG.txt", changelog_info);
        changelog_info.push_back(0);
    }
    if (line_size.x == 0) {
        line_size
            = ImGui::CalcTextSize("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                  "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    }
    if (!_show_changelog) {
        return;
    }
    std::string title = std::string { _("Changelog") } + "###Changelog";
    ImGui::OpenPopup(title.c_str());
    if (ImGui::BeginPopupModal(title.c_str(), &_show_changelog,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::BeginChild(
            "License", ImVec2(line_size.x, 450), ImGuiChildFlags_FrameStyle);
        ImGui::InputTextMultiline("##LicenseText", (char*)changelog_info.data(),
            changelog_info.size(), ImGui::GetContentRegionAvail(),
            ImGuiInputTextFlags_ReadOnly);
        ImGui::EndChild();
        ImGui::EndPopup();
    }
}

static void _color_buttons(const Theme& style)
{
    ImGui::PushID(style.name.c_str());
    ImGui::ColorButton(_("Background"), style.bg);
    ImGui::SameLine();
    ImGui::ColorButton(_("Foreground"), style.fg);

    ImGui::ColorButton(_("Black"), style.black);
    ImGui::SameLine();
    ImGui::ColorButton(_("Red"), style.red);
    ImGui::SameLine();
    ImGui::ColorButton(_("Green"), style.green);
    ImGui::SameLine();
    ImGui::ColorButton(_("Yellow"), style.yellow);

    ImGui::ColorButton(_("Black Bright"), style.black_bright);
    ImGui::SameLine();
    ImGui::ColorButton(_("Red Bright"), style.red_bright);
    ImGui::SameLine();
    ImGui::ColorButton(_("Green Bright"), style.green_bright);
    ImGui::SameLine();
    ImGui::ColorButton(_("Yellow Bright"), style.yellow_bright);

    ImGui::ColorButton(_("Blue"), style.blue);
    ImGui::SameLine();
    ImGui::ColorButton(_("Magenta"), style.magenta);
    ImGui::SameLine();
    ImGui::ColorButton(_("Cyan"), style.cyan);
    ImGui::SameLine();
    ImGui::ColorButton(_("White"), style.white);

    ImGui::ColorButton(_("Blue Bright"), style.blue_bright);
    ImGui::SameLine();
    ImGui::ColorButton(_("Magenta Bright"), style.magenta_bright);
    ImGui::SameLine();
    ImGui::ColorButton(_("Cyan Bright"), style.cyan_bright);
    ImGui::SameLine();
    ImGui::ColorButton(_("White Bright"), style.white_bright);
    ImGui::PopID();
}
void _popup_pref(void)
{
    static int light_idx = 0;
    static int dark_idx  = 0;
    static int lang_idx  = -1;
    if (lang_idx == -1) {
        for (size_t i = 0; i < fs::localsize(); i++) {
            if (cfg.language == fs::locales()[i]) {
                lang_idx = i;
                break;
            }
        }
    }
    static const ImVec2 __table_l_size
        = ImGui::CalcTextSize("LIGHT THEME STYLE");
    const char* pref_table[] = {
        _("Follow Device"),
        _("Light Theme"),
        _("Dark Theme"),
    };
    if (!_show_pref) {
        return;
    }
    auto light_themes = get_available_styles(false);
    auto dark_themes  = get_available_styles(true);
    if (cfg.is_saved && cfg.is_applied) {
        cfg = Configuration::get();
        for (size_t i = 0; i < light_themes.size(); i++) {
            if (cfg.light_theme == light_themes[i]) {
                light_idx = i;
            }
        }
        for (size_t i = 0; i < dark_themes.size(); i++) {
            if (cfg.dark_theme == dark_themes[i]) {
                dark_idx = i;
            }
        }
    }
    bool keep         = true;
    std::string title = std::string { _("Preferences") } + "###Preferences";
    ImGui::Begin(title.c_str(), &keep,
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoDocking
            | (!Configuration::get().is_saved ? ImGuiWindowFlags_UnsavedDocument
                                              : ImGuiWindowFlags_None));

    ImGui::SetWindowPos(
        ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing);
    Section(_("Appearance"));
    AnonTable(
        "AppearanceTable", __table_l_size.x,
        TablePair(
            Field(_("UI Scale")),
            if (ImGui::SliderInt("%", &cfg.scale, 75, 150)) {
                cfg.is_applied = false;
            });
        ImGui::BeginDisabled(fs::localsize() == 0); TablePair(
            Field(_("Language")),
            if (ImGui::Combo("##Language", &lang_idx, fs::localnames(),
                    fs::localsize())) {
                cfg.is_applied = false;
                cfg.language   = fs::locales()[lang_idx];
            });
        ImGui::EndDisabled(); TablePair(
            Field(_("Light Theme Style")),
            if (ImGui::Combo("##Select Theme", &light_idx, light_themes.data(),
                    light_themes.size())) {
                cfg.is_applied  = false;
                cfg.light_theme = light_themes[light_idx];
            });
        _color_buttons(ui::get_theme(cfg.light_theme)); TablePair(
            Field(_("Dark Theme Style")),
            if (ImGui::Combo("##Select Theme Dark", &dark_idx,
                    dark_themes.data(), dark_themes.size())) {
                cfg.is_applied = false;
                cfg.dark_theme = dark_themes[dark_idx];
            });
        _color_buttons(ui::get_theme(cfg.dark_theme)); TablePair(
            Field(_("Rounded Corners")),
            if (ImGui::SliderInt("##Rounded Corners", &cfg.rounded_corners, 0,
                    20)) { cfg.is_applied = false; });
        TablePair(
            Field(_("Theme Preference")),
            if (ImGui::Combo("##ThemePreference", (int*)&cfg.preference,
                    pref_table, 3)) { cfg.is_applied = false; });
        TablePair(
            Field(_("Start in Fullscreen")),
            if (ImGui::Checkbox("##Fullscreen", &cfg.start_fullscreen)) {
                cfg.is_applied = false;
            });
        ImGui::BeginDisabled(cfg.start_fullscreen);
        TableKey(Field(_("Startup Window")));
        Point p = { static_cast<int16_t>(cfg.startup_win_x),
            static_cast<int16_t>(cfg.startup_win_y) };
        if (Point newp = PositionSelector(p, "##StartupWindow"); newp != p) {
            cfg.startup_win_x = newp.x;
            cfg.startup_win_y = newp.y;
            cfg.is_applied    = false;
        };
        ImGui::EndDisabled();

    );
    EndSection();
    ImGuiIO& imio = ImGui::GetIO();
    ImGui::Text(
        "FPS: %4.f Uptime: %4.f seconds", imio.Framerate, ImGui::GetTime());
    ImGui::BeginDisabled(cfg.is_applied);
    if (IconButton(ICON_LC_REDO_DOT, _("Apply"))) {
        cfg.is_saved = false;
        Configuration::set(cfg);
        cfg            = Configuration::get();
        cfg.is_applied = true;
        Toast(ICON_LC_SETTINGS_2, _("Preferences"),
            _("Configuration changes were applied."));
    }
    ImGui::EndDisabled();
    ImGui::SameLine(ImGui::GetWindowSize().x * 5 / 6);
    ImGui::BeginDisabled(cfg.is_saved);
    if (IconButton(ICON_LC_SAVE, _("Save"))) {
        Configuration::set(cfg);
        cfg = Configuration::get();
        Configuration::save();
        cfg.is_applied = true;
        cfg.is_saved   = true;
        L_DEBUG("Configuration changes were saved.");
        Toast(ICON_LC_SAVE, _("Preferences"),
            _("Configuration changes were saved."));
    }
    ImGui::EndDisabled();
    ImGui::End();
    if (!keep) {
        if (!Configuration::get().is_saved) {
            L_DEBUG("Configuration changes were reverted.");
            Toast(ICON_LC_UNDO, _("Preferences"),
                _("Configuration changes were reverted."));
            Configuration::set(Configuration::load());
            cfg = Configuration::get();
        }
        _show_pref = false;
    }
}

} // namespace ic::ui
