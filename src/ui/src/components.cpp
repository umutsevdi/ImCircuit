#include <imnodes.h>

#include "components.h"
#include "core.h"
#include "imgui.h"

namespace ic::ui {

Point PositionSelector(Point point, const char* prefix)
{
    const static ImVec2 _SELECTOR_SIZE = ImGui::CalcTextSize("-000000000000");
    int x = point.x, y = point.y;
    std::string s_prefix = "##";
    s_prefix += prefix;
    ImGui::AlignTextToFramePadding();
    ImGui::Text("x");
    ImGui::SameLine();
    ImGui::PushItemWidth(_SELECTOR_SIZE.x);
    ImGui::InputInt("##x", &x);
    ImGui::Text("y");
    ImGui::SameLine();
    ImGui::InputInt("##y", &y);
    ImGui::PopItemWidth();
    return { static_cast<int16_t>(x), static_cast<int16_t>(y) };
}

void ToggleButton(Input& node)
{
    const Theme& style = get_active_style();
    switch (node.get()) {
    case State::TRUE:
        ImGui::PushStyleColor(ImGuiCol_Button, style.green);
        ImGui::PushStyleColor(ImGuiCol_Text, style.fg);
        if (ImGui::Button(to_str<State>(State::TRUE))) {
            node.toggle();
        };
        ImGui::PopStyleColor();
        break;
    case State::FALSE:
        ImGui::PushStyleColor(ImGuiCol_Button, style.red);
        if (ImGui::Button(to_str<State>(State::FALSE))) {
            node.toggle();
        };
        break;
    default: ImGui::PushStyleColor(ImGuiCol_Button, style.gray); break;
    }
    ImGui::PopStyleColor();
}

void NodeTypeTitle(Node n)
{
    static char buffer[256];
    snprintf(buffer, 256, "%s@%u", to_str<Node::Type>(n.type), n.index);
    if (ImGui::TextLink(buffer)) {
        ImNodes::ClearNodeSelection();
        switch (n.type) {
        case Node::Type::COMPONENT_INPUT:
        case Node::Type::COMPONENT_OUTPUT:
            ImNodes::SelectNode(Node { 0, n.type }.numeric());
            break;
        default: ImNodes::SelectNode(n.numeric()); break;
        }
    };
}

void NodeTypeTitle(Node n, sockid)
{
    ImGui::PushFont(get_font(REGULAR));
    NodeTypeTitle(n);
    ImGui::PopFont();
}
} // namespace ic::ui
