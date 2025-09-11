#include <imnodes.h>

#include "components.h"
#include "core.h"

namespace lcs::ui {

Point PositionSelector(Point point, const char* prefix)
{
    const static ImVec2 _SELECTOR_SIZE = ImGui::CalcTextSize("-000000000000");

    std::string s_prefix = "##";
    s_prefix += prefix;
    ImGui::AlignTextToFramePadding();
    ImGui::Text("x");
    ImGui::SameLine();
    ImGui::PushItemWidth(_SELECTOR_SIZE.x);
    int x = point.x, y = point.y;
    ImGui::Text("y");
    ImGui::SameLine();
    ImGui::PopItemWidth();
    return { static_cast<int16_t>(x), static_cast<int16_t>(y) };
}

State ToggleButton(State state, bool clickable)
{
    const LcsTheme& style = get_active_style();
    ImGui::PushFont(get_font(FontFlags::BOLD), FONT_SMALL);
    switch (state) {
    case State::TRUE:
        if (clickable) {
            ImGui::PushStyleColor(ImGuiCol_Button, style.green);
            ImGui::PushStyleColor(ImGuiCol_Text, style.fg);
            if (ImGui::Button(to_str<State>(State::TRUE))) {
                state = FALSE;
            };
            ImGui::PopStyleColor();
        } else {
            ImGui::TextColored(style.green, "%s", to_str<State>(State::TRUE));
        }
        break;
    case State::FALSE:
        if (clickable) {
            ImGui::PushStyleColor(ImGuiCol_Button, style.red);
            if (ImGui::Button(to_str<State>(State::FALSE))) {
                state = TRUE;
            };
        } else {
            ImGui::TextColored(style.red, "%s", to_str<State>(State::FALSE));
        }
        break;
    case State::DISABLED:
        if (clickable) {
            ImGui::PushStyleColor(ImGuiCol_Button, style.black_bright);
            ImGui::PushStyleColor(ImGuiCol_Text, style.white_bright);
            ImGui::Button(_(to_str<State>(State::DISABLED)));
            ImGui::PopStyleColor();
        } else {
            ImGui::TextColored(
                style.black_bright, "%s", to_str<State>(State::DISABLED));
        }
        break;
    }
    if (clickable) {
        ImGui::PopStyleColor();
    }
    ImGui::PopFont();
    return state;
}

void ToggleButton(Ref<Input> node)
{
    State s_old = node->get();

    if (s_old != ToggleButton(node->get(), true)) {
        node->toggle();
    }
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
    ImGui::PushFont(get_font(FontFlags::REGULAR));
    NodeTypeTitle(n);
    ImGui::PopFont();
}
} // namespace lcs::ui
