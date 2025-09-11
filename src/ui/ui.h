#pragma once
/*******************************************************************************
 * \file
 * File: ui/ui.h
 * Created: 08/12/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include <imgui.h>
#include "core.h"
namespace lcs::ui {
void before(void);
void after(ImGuiIO& io);
bool loop(ImGuiIO& io);
void RenderNotifications(void);

inline ImVec4 v4mul(ImVec4 vec, float pct)
{
    return ImVec4(vec.x * pct, vec.y * pct, vec.z * pct, vec.w);
}

inline ImVec4 v4mul(ImVec4 vec, float pct, float alpha)
{
    return ImVec4(vec.x * pct, vec.y * pct, vec.z * pct, vec.w * alpha);
}

void set_style(ImGuiIO& io, bool init = false);

namespace layout {
    void MenuBar(void);
    void Palette(Ref<Scene>);
    void Inspector(Ref<Scene>);
    void NodeEditor(Ref<Scene> scene, bool switched);
    int _input_text_callback(ImGuiInputTextCallbackData*, bool switched);
    void Profile(std::string& name, bool switched);
    void SceneInfo(Ref<Scene>, bool switched);
    void Console(void);
    void DebugWindow(Ref<Scene>);
    void PropertyEditor(Ref<Scene>);
} // namespace layout

namespace popup {
    void LoginWindow(bool& df_show);
} // namespace popup

namespace dialog {
    Error open_file(void);
    Error save_file_as(void);
} // namespace dialog
} // namespace lcs::ui
