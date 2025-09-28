#include <IconsLucide.h>
#include <clocale>
#include <filesystem>
#include <imgui_internal.h>
#include <json/reader.h>
#include <json/value.h>
#include <libintl.h>
#include "common.h"
#include "ui.h"

namespace ic {
template <>
const char* to_str<ui::Configuration::Preference>(
    ui::Configuration::Preference t)
{
    switch (t) {
    case ui::Configuration::ALWAYS_LIGHT: return "light";
    case ui::Configuration::ALWAYS_DARK: return "dark";
    default: return "os";
    }
}

namespace ui {
    static Error _set_locale(const std::string& locale);
    static Json::Value _to_json(const Configuration& cfg);
    LCS_ERROR static _from_json(const Json::Value& v, Configuration& cfg);

    static Configuration ACTIVE_CONFIG;

    static Configuration::Preference _str_to_pref(const std::string& s)
    {
        if (s == "light") {
            return Configuration::ALWAYS_LIGHT;
        } else if (s == "dark") {
            return Configuration::ALWAYS_DARK;
        }
        return Configuration::FOLLOW_OS;
    }

    Configuration& Configuration::load(const std::filesystem::path& path)
    {
        std::string old_locale = ACTIVE_CONFIG.language;
        std::string data;
        if (!fs::read(path, data)) {
            L_WARN("Configuration file was not found at %s. Initializing "
                   "defaults.",
                (path).c_str());
            fs::write(path, _to_json(ACTIVE_CONFIG).toStyledString());
            return ACTIVE_CONFIG;
        }

        Json::Value v;
        Json::Reader r;
        if (!r.parse(data, v)) {
            L_ERROR("Invalid configuration file format. Overwriting.");
            fs::write(path, _to_json(ACTIVE_CONFIG).toStyledString());
            return ACTIVE_CONFIG;
        }
        if (_from_json(v, ACTIVE_CONFIG) != Error::OK) {
            L_ERROR("Parse error.");
            ACTIVE_CONFIG = Configuration();
        }
        if (old_locale != ACTIVE_CONFIG.language) {
            _set_locale(ACTIVE_CONFIG.language);
        }
        L_DEBUG("Configuration was loaded.");
        return ACTIVE_CONFIG;
    }

    Configuration& Configuration::get(void) { return ACTIVE_CONFIG; }

    void Configuration::set(Configuration& cfg)
    {
        if (cfg.language != ACTIVE_CONFIG.language) {
            if (_set_locale(cfg.language)) {
                cfg.language = ACTIVE_CONFIG.language;
            }
        }
        ACTIVE_CONFIG            = cfg;
        ACTIVE_CONFIG.is_applied = false;
        ACTIVE_CONFIG.is_saved   = false;
    }

    void Configuration::save(void)
    {
        fs::write(fs::CONFIG / "config.json",
            _to_json(ACTIVE_CONFIG).toStyledString());
        ACTIVE_CONFIG.is_applied = true;
        ACTIVE_CONFIG.is_saved   = true;
    }

    static Json::Value _to_json(const Configuration& cfg)
    {
        Json::Value v;
        v["theme"]["light"]       = cfg.light_theme;
        v["theme"]["dark"]        = cfg.dark_theme;
        v["theme"]["prefer"]      = to_str(cfg.preference);
        v["theme"]["corners"]     = cfg.rounded_corners;
        v["scale"]                = cfg.scale;
        v["language"]             = cfg.language;
        v["window"]["x"]          = cfg.startup_win_x;
        v["window"]["y"]          = cfg.startup_win_y;
        v["proxy"]                = cfg.api_proxy;
        v["window"]["fullscreen"] = cfg.start_fullscreen;
        if (!cfg.login.empty()) {
            v["service"]["login"] = cfg.login;
        }
        return v;
    }

    LCS_ERROR static _from_json(const Json::Value& v, Configuration& cfg)
    {
        if (!(v["theme"].isObject()
                && (v["theme"]["light"].isString()
                    && v["theme"]["dark"].isString()
                    && v["theme"]["prefer"].isString()
                    && v["theme"]["corners"].isUInt())
                && v["scale"].isInt() && v["language"].isString()
                && v["window"].isObject()
                && (v["window"]["x"].isInt() && v["window"]["y"].isInt()
                    && v["window"]["fullscreen"].isBool())
                && v["proxy"].isString())) {
            return ERROR(INVALID_JSON);
        }

        cfg.light_theme      = v["theme"]["light"].asString();
        cfg.dark_theme       = v["theme"]["dark"].asString();
        cfg.preference       = _str_to_pref(v["theme"]["prefer"].asString());
        cfg.rounded_corners  = v["theme"]["corners"].asInt();
        cfg.scale            = v["scale"].asInt();
        cfg.api_proxy        = v["proxy"].asString();
        cfg.language         = v["language"].asString();
        cfg.startup_win_x    = v["window"]["x"].asInt();
        cfg.startup_win_y    = v["window"]["y"].asInt();
        cfg.start_fullscreen = v["window"]["fullscreen"].asBool();
        if (v["service"].isObject() && v["service"]["login"].isString()) {
            cfg.login = cfg.login;
        }
        cfg.is_saved = true;

        if (!(!cfg.light_theme.empty() && !cfg.dark_theme.empty()
                && (cfg.rounded_corners >= 0 && cfg.rounded_corners <= 20)
                && (cfg.scale >= 75 && cfg.scale <= 150))) {
            return ERROR(INVALID_JSON);
        }
        return Error::OK;
    }

    static Error _set_locale(const std::string& locale)
    {
        L_DEBUG("Updating locale to %s", locale.c_str());
        const char* domain = APPNAME_BIN;
        std::string dir    = fs::LOCALE.string();
        if (!(bindtextdomain(domain, dir.c_str())
                && bind_textdomain_codeset(domain, "UTF-8"))) {
            return ERROR(Error::LOCALE_ERROR);
        }
        if (setlocale(LC_ALL, (locale + ".UTF-8").c_str()) == nullptr) {
            return ERROR(Error::LOCALE_ERROR);
        }
        if (
#ifdef _WIN32
            _putenv_s("LC_ALL", locale.c_str())
#else
            setenv("LC_ALL", locale.c_str(), 1)
#endif
        ) {
            return ERROR(Error::LOCALE_ERROR);
        }
        if (!textdomain(domain)) {
            return ERROR(Error::LOCALE_ERROR);
        };
        return Error::OK;
    }

    void bind_config(ImGuiContext* ctx)
    {
        ImGuiSettingsHandler handler {};
        handler.TypeName   = APPNAME;
        handler.TypeHash   = ImHashStr(APPNAME);
        handler.ReadOpenFn = [](ImGuiContext*, ImGuiSettingsHandler*,
                                 const char* name) -> void* {
            if (std::strncmp(name, "default", sizeof("default")) == 0) {
                return &ACTIVE_CONFIG;
            }
            return nullptr;
        };
        handler.ReadLineFn = [](ImGuiContext*, ImGuiSettingsHandler*, void*,
                                 const char* line) {
            char window_name[40];
            char is_active[6];
            if (sscanf(line, "%39[^\"]=%5[^\"]", window_name, is_active) == 2) {
                bool value = false;
                if (strncmp(is_active, "true", 4) == 0) {
                    value = true;
                }
                for (auto& win : WINDOW_LIST) {
                    if (strncmp(win->basename, window_name, 40) == 0) {
                        L_INFO("??");
                        win->is_active = value;
                        break;
                    }
                }
            }
        };
        handler.WriteAllFn
            = [](ImGuiContext*, ImGuiSettingsHandler*, ImGuiTextBuffer* buf) {
                  buf->appendf("[%s][%s]\n", APPNAME, "default");
                  for (auto& win : WINDOW_LIST) {
                      buf->appendf("%s=%s\n", win->basename,
                          win->is_active ? "true" : "false");
                  }
              };
        handler.ApplyAllFn = [](ImGuiContext*, ImGuiSettingsHandler*) { };
        handler.UserData   = nullptr;
        ctx->SettingsHandlers.push_back(handler);
        L_DEBUG("Bind .ini completed.");
    }

} // namespace ui
} // namespace ic
