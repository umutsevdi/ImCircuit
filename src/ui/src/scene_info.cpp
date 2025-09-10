#include <IconsLucide.h>
#include "components.h"
#include "configuration.h"
#include "imgui.h"
#include "ui.h"

namespace lcs::ui ::layout {
static std::array<char, 128> _name {};
static std::array<char, 512> _description {};
void SceneInfo(Ref<Scene> scene, bool switched)
{
    if (!user_data.scene_info) {
        return;
    }
    std::string title
        = std::string { _("Scene Information") } + "###SceneInformation";
    if (ImGui::Begin(title.c_str(), &user_data.scene_info)) {
        HINT("CTRL+N", _("Scene Information"),
            "A management panel for updating and configuring the active\n"
            "scene's details. Use this menu to handle dependencies and share\n"
            "your scene with others.");
        ImGui::BeginDisabled(scene == nullptr);
        if (scene != nullptr && switched) {
            _name        = scene->name();
            _description = scene->description();
        }
        Section(_("Scene"));
        Field(_("Scene Name"));
        if (scene != nullptr
            && ImGui::InputText("##SceneNameInputText", _name.data(),
                _name.max_size(), ImGuiInputTextFlags_CharsNoBlank)) {
            scene->set_name(_name.data());
        };
        Field(_("Author"));
        if (scene != nullptr) {
            ImGui::TextUnformatted(scene->author().data());
            Field(_("Version"));
            ImGui::Text("%d", scene->version);
        }
        Field(_("Description"));
        if (scene != nullptr
            && ImGui::InputTextMultiline("##SceneDescInputText",
                _description.data(), _description.max_size(),
                ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            scene->set_name(_description.data());
        };
        EndSection();

        if (scene != nullptr && scene->component_context.has_value()) {
            Section(_("Component Attributes"));
            Field(_("Input Size"));
            size_t input_size  = scene->component_context->inputs.size();
            size_t output_size = scene->component_context->outputs.size();
            ImGui::InputInt("##CompInputSize", (int*)&input_size);
            Field(_("Output Size"));
            ImGui::InputInt("##CompOutputSize", (int*)&output_size);

            if (input_size != scene->component_context->inputs.size()
                || output_size != scene->component_context->outputs.size()) {
                scene->component_context->setup(input_size, output_size);
            }
            EndSection();
        }
        if (scene != nullptr && !scene->dependencies().empty()) {
            Section(_("Dependencies"));
            if (ImGui::BeginTable(_("Dependencies"), 4,
                    ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg)) {
                ImGui::TableHeader("Dependencies");
                ImGui::TableSetupColumn(
                    _("Author"), ImGuiTableColumnFlags_WidthFixed);
                ImGui::NextColumn();
                ImGui::TableSetupColumn(
                    _("Name"), ImGuiTableColumnFlags_WidthStretch);
                ImGui::NextColumn();
                ImGui::TableSetupColumn(
                    _("Version"), ImGuiTableColumnFlags_WidthFixed);
                ImGui::NextColumn();
                ImGui::TableSetupColumn(
                    _("Number of Nodes"), ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();
                for (size_t i = 0; i < scene->dependencies().size(); i++) {
                    const auto& dep = scene->dependencies()[i];
                    bool selected   = false;
                    ImGui ::TableNextRow();
                    ImGui ::TableSetColumnIndex(0);
                    ImGui::Selectable(("##" + std::to_string(i)).c_str(),
                        &selected, ImGuiSelectableFlags_SpanAllColumns);
                    ImGui::SameLine();
                    ImGui::TextUnformatted(dep.name().data());
                    ImGui ::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(dep.author().data());
                    ImGui ::TableSetColumnIndex(2);
                    ImGui::Text("%d", dep.version);
                    ImGui ::TableSetColumnIndex(3);
                    int count = 0;
                    // FIXME
                    //                  for (auto& x : scene->_components) {
                    //                      if (x.path == d) {
                    //                          count++;
                    //                      }
                    //                  }
                    ImGui::Text("%d", count);
                    if (selected) {
                        Toast(ICON_LC_CLIPBOARD_COPY, _("Clipboard"),
                            _("Dependency name is copied to the clipboard."));
                        ImGui::SetClipboardText(dep.to_dependency().c_str());
                    }
                }
                ImGui::EndTable();
                IconButton(ICON_LC_PACKAGE, _("Add Component"));
            }
            EndSection();
        }
        if (IconButton(ICON_LC_UPLOAD, _("Upload"))) {
            std::string resp;
            // net::upload_scene(&scene, resp);
        }
        ImGui::EndDisabled();
    }
    ImGui::End();
}

} // namespace lcs::ui::layout
