#include <imgui.h>
#include <imnodes.h>
#include "IconsLucide.h"
#include "components.h"
#include "core.h"
#include "ui.h"

namespace ic::ui {

struct Properties final : public Window {
    Properties();
    ~Properties() = default;

    virtual void show(Ref<Scene>, bool switched) override;
    virtual const char* tooltip(void) override
    {
        return _("Searchable object tree.");
    }

private:
    ImGuiTextFilter Filter;

    void _show_rel(Ref<Scene> scene, relid id);
    void _show_input(Ref<Scene> scene, Node id);
    void _show_output(Ref<Scene> scene, Node id);
    void _show_component(Ref<Scene> scene, Node id);
    void _show_gate(Ref<Scene> scene, Node id);
    void _show_node(Ref<Scene> scene, Node id);
};
REGISTER_WINDOW("Properties", Properties);

void Properties::show(Ref<Scene> scene, bool)
{
    IconText(ICON_LC_SEARCH, FONT_NORMAL, "");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::InputText("##Filter", Filter.InputBuf,
            IM_ARRAYSIZE(Filter.InputBuf),
            ImGuiInputTextFlags_EscapeClearsAll
                | ImGuiInputTextFlags_AutoSelectAll)) {
        Filter.Build();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_None)) {
        BeginTooltip(ICON_LC_SEARCH, _("Search Box"));
        ImGui::TextUnformatted(
            _("Search among nodes. You can filter multiple nodes separated "
              "by\ncommas; a leading ‑ excludes a node."));
        ImGui::EndTooltip();
    }
    ImGui::BeginChild("##frame", ImVec2(), ImGuiChildFlags_Borders);
    if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg)) {
        if (scene != nullptr) {
            for (uint16_t i = 0; i < scene->_gates.size(); i++) {
                std::string name
                    = std::string { to_str<Node::Type>(Node::GATE) } + "@"
                    + std::to_string(i);
                if ((!Filter.IsActive() || Filter.PassFilter(name.c_str()))
                    && !scene->_gates[i].is_null()) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    Node node { i, Node::GATE };
                    if (ImGui::TreeNode(name.c_str())) {
                        if (!ImNodes::IsNodeSelected(node.numeric())) {
                            ImNodes::SelectNode(node.numeric());
                        }
                        _show_gate(scene, node);
                        ImGui::TreePop();
                    }
                }
            }
            for (uint16_t i = 0; i < scene->_inputs.size(); i++) {
                std::string name
                    = std::string { to_str<Node::Type>(Node::INPUT) } + "@"
                    + std::to_string(i);
                if ((!Filter.IsActive() || Filter.PassFilter(name.c_str()))
                    && !scene->_inputs[i].is_null()) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    Node node { i, Node::INPUT };
                    if (ImGui::TreeNode(name.c_str())) {
                        if (!ImNodes::IsNodeSelected(node.numeric())) {
                            ImNodes::SelectNode(node.numeric());
                        }
                        _show_input(scene, node);
                        ImGui::TreePop();
                    }
                }
            }
            for (uint16_t i = 0; i < scene->_outputs.size(); i++) {
                std::string name
                    = std::string { to_str<Node::Type>(Node::OUTPUT) } + "@"
                    + std::to_string(i);
                if ((!Filter.IsActive() || Filter.PassFilter(name.c_str()))
                    && !scene->_outputs[i].is_null()) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    Node node { i, Node::OUTPUT };
                    if (ImGui::TreeNode(name.c_str())) {
                        if (!ImNodes::IsNodeSelected(node.numeric())) {
                            ImNodes::SelectNode(node.numeric());
                        }
                        _show_output(scene, node);
                        ImGui::TreePop();
                    }
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}

void Properties::_show_input(Ref<Scene> scene, Node id)
{
    auto input = scene->get_node<Input>(id);
    ImGui::BeginGroup();
    ImGui::BulletText(_("Value: %s"), to_str<State>(input->get()));
    if (input->output.empty()) {
        ImGui::BulletText(_("Output"));
    } else {
        if (ImGui::TreeNode(_("Output"))) {
            for (size_t i = 0; i < input->output.size(); i++) {
                if (ImGui::TreeNode(std::to_string(i).c_str())) {
                    _show_rel(scene, input->output[i]);
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        }
    }
    ImGui::EndGroup();
}
void Properties::_show_output(Ref<Scene> scene, Node id)
{
    auto output = scene->get_node<Output>(id);
    ImGui::BeginGroup();
    ImGui::BulletText(_("Value: %s"), to_str<State>(output->get()));

    if (output->input) {
        if (ImGui::TreeNode(_("Input"))) {
            if (output->input != 0) {
                _show_rel(scene, output->input);
            }
            ImGui::TreePop();
        }
    } else {
        ImGui::BulletText(_("Input"));
    }
    ImGui::EndGroup();
}

void Properties::_show_component(Ref<Scene> scene, Node id) { }

void Properties::_show_gate(Ref<Scene> scene, Node id)
{
    auto gate = scene->get_node<Gate>(id);
    ImGui::BeginGroup();
    ImGui::BulletText(_("Type: %s"), to_str<Gate::Type>(gate->type()));
    if (ImGui::TreeNode(_("Inputs"))) {
        for (size_t i = 0; i < gate->inputs.size(); i++) {
            if (gate->inputs[i] == 0) {
                ImGui::BulletText("%zu", i);
            } else if (ImGui::TreeNode(std::to_string(i).c_str())) {
                _show_rel(scene, gate->inputs[i]);
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(_("Output"))) {
        for (size_t i = 0; i < gate->output.size(); i++) {
            _show_rel(scene, gate->output[i]);
        }
        ImGui::TreePop();
    }
    ImGui::EndGroup();
}

void Properties::_show_node(Ref<Scene> scene, Node id)
{
    switch (id.type) {
    case Node::GATE: _show_gate(scene, id); break;
    case Node::COMPONENT: _show_component(scene, id); break;
    case Node::INPUT: _show_input(scene, id); break;
    case Node::OUTPUT: _show_output(scene, id); break;
    default: break;
    }
}

void Properties::_show_rel(Ref<Scene> scene, relid id)
{
    if (id == 0) {
        return;
    }
    auto rel = scene->get_rel(id);
    ImGui::BeginGroup();
    ImGui::BulletText(_("Value: %s"), to_str<State>(rel->value));
    if (ImGui::TreeNode(_("From"))) {
        _show_node(scene, rel->from_node);
        ImGui::BulletText(_("Socket %d"), rel->from_sock);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(_("To"))) {
        _show_node(scene, rel->to_node);
        ImGui::BulletText(_("Socket %d"), rel->to_sock);
        ImGui::TreePop();
    }
    ImGui::EndGroup();
}
} // namespace ic::ui
