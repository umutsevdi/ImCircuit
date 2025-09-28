#include <IconsLucide.h>
#include <cmath>
#include <imnodes.h>
#include "components.h"
#include "core.h"
#include "imgui.h"
#include "ui.h"

namespace ic::ui {

struct Editor final : public Window {
    enum MenuType { NODE, LINK, NEW };
    Editor();
    ~Editor() = default;

    virtual void show(Ref<Scene>, bool switched) override;
    virtual const char* tooltip(void) override
    {
        return _("A Panel to edit nodes in the selected scene.");
    }

    MenuType menu  = NODE;
    bool is_active = false;

private:
    void _show_node(Input& node, uint16_t id, bool is_changed);
    void _show_node(Output& node, uint16_t id, bool is_changed);
    void _show_node(Gate& node, uint16_t id, bool is_changed);
    void _show_node(Component& node, uint16_t id, bool is_changed);
    void _show_node(ComponentContext& node, uint16_t, bool);
    void _sync_position(BaseNode& node, uint32_t node_id, bool is_changed);

    void _context_menu_new(Ref<Scene>);

    inline ImNodesPinShape_ to_shape(bool value, bool is_input)
    {
        if (is_input) {
            return value ? ImNodesPinShape_TriangleFilled
                         : ImNodesPinShape_Triangle;
        }
        return value ? ImNodesPinShape_QuadFilled : ImNodesPinShape_Quad;
    }

    inline bool _is_mouse_in(void)
    {
        ImVec2 mouse = ImGui::GetMousePos();
        ImVec2 wmin  = ImGui::GetWindowPos();
        ImVec2 wmax { wmin.x + ImGui::GetWindowSize().x,
            wmin.y + ImGui::GetWindowSize().y };
        return mouse.x > wmin.x && mouse.y > wmin.y && mouse.x < wmax.x
            && mouse.y < wmax.y;
    }

    bool is_copied = false;
    ImVec2 copy_node_position;
    int nodeids[1 << 20] = { 0 };
};
REGISTER_WINDOW("Editor", Editor, {
    _flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoNavFocus;
});

void Editor::show(Ref<Scene> scene, bool is_changed)
{
    ImNodes::BeginNodeEditor();
    if (scene != nullptr) {
        const Theme& style = get_active_style();
        if (scene->component_context.has_value()) {
            _show_node(scene->component_context.value(), 0, is_changed);
        }
        for (size_t i = 0; i < scene->_inputs.size(); i++) {
            if (!scene->_inputs[i].is_null()) {
                _show_node(scene->_inputs[i], i, is_changed);
            }
        }
        for (size_t i = 0; i < scene->_outputs.size(); i++) {
            if (!scene->_outputs[i].is_null()) {
                _show_node(scene->_outputs[i], i, is_changed);
            }
        }
        for (size_t i = 0; i < scene->_gates.size(); i++) {
            if (!scene->_gates[i].is_null()) {
                _show_node(scene->_gates[i], i, is_changed);
            }
        }
        for (size_t i = 0; i < scene->_components.size(); i++) {
            if (!scene->_components[i].is_null()) {
                _show_node(scene->_components[i], i, is_changed);
            }
        }
        for (auto& r : scene->_relations) {

            ImNodes::PushColorStyle(ImNodesCol_Link,
                r.second.value == State::TRUE ? ImGui::GetColorU32(style.green)
                    : r.second.value == State::FALSE
                    ? ImGui::GetColorU32(style.red)
                    : ImGui::GetColorU32(style.gray));
            ImNodes::Link(r.first,
                encode_pair(r.second.from_node, r.second.from_sock, true),
                encode_pair(r.second.to_node, r.second.to_sock, false));
            ImNodes::PopColorStyle();
        }
        ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_TopRight);
        ImNodes::EndNodeEditor();

        int linkid = 0;
        if (ImNodes::IsLinkHovered(&linkid)) {
            if (auto r = scene->get_rel(linkid); r != nullptr
                && BeginTooltip(ICON_LC_CABLE, _("Relation %d"), linkid)) {
                AnonTable("Link", 0,
                    TablePair(Field(_("From")),
                        NodeTypeTitle(r->from_node, r->from_sock));
                    TablePair(
                        Field(_("To")), NodeTypeTitle(r->to_node, r->to_sock));
                    TablePair(Field(_("Value")),
                        ImGui::Text("%s", to_str(r->value))););
                EndTooltip();
            }
        }

        int nodeid_encoded = 0;
        if (ImNodes::IsNodeHovered(&nodeid_encoded)) {
            Node nodeid { static_cast<uint16_t>(0xFFFF & nodeid_encoded),
                (Node::Type)(nodeid_encoded >> 16) };
            if (auto n = scene->get_base(nodeid); n != nullptr
                && BeginTooltip(ICON_LC_CODESANDBOX, _("Node %s@%d"),
                    to_str<Node::Type>(nodeid.type), nodeid.index)) {
                AnonTable(
                    "Node", 0,
                    TablePair(Field(_("Position")),
                        ImGui::Text("(%d, %d)", n->point().x, n->point().y));
                    TableKey(Field(_("Value")));
                    if (nodeid.type != Node::Type::COMPONENT) {
                        ImGui::Text("%s", to_str(n->get()));
                    } else {
                        auto comp = scene->get_node<Component>(nodeid);
                        ImGui::Text("(");
                        ImGui::SameLine();
                        for (size_t i = 0; i < comp->outputs.size(); i++) {
                            ImGui::Text("%s", to_str(comp->get(i)));
                            ImGui::SameLine();
                        }
                        ImGui::Text(")");
                    } if (!n->is_connected()) {
                        ImGui::PushFont(nullptr, FONT_SMALL);
                        ImGui::TextColored(get_active_style().red,
                            _("Please connect all pins."));
                        ImGui::PopFont();
                    });
                EndTooltip();
            }
        }

        int pin_id = 0;
        if (ImNodes::IsPinHovered(&pin_id)) {
            bool is_out = false;
            sockid sock = 0;
            Node nodeid = decode_pair(pin_id, &sock, &is_out);

            sock = nodeid.type == Node::Type::COMPONENT_INPUT
                    || nodeid.type == Node::Type::COMPONENT_OUTPUT
                ? nodeid.index
                : sock;
            if (BeginTooltip(ICON_LC_HEXAGON, _("%s Socket %u"),
                    is_out ? _("Output") : _("Input"), sock + 1)) {
                AnonTable(
                    "Node", 0,
                    TablePair(Field(_("Owner")), NodeTypeTitle(nodeid));
                    if (is_out) {
                        TableKey(Field(_("Value")));
                        if (nodeid.type == Node::Type::COMPONENT_INPUT
                            || nodeid.type == Node::Type::COMPONENT_OUTPUT) {
                            ImGui::Text("%s",
                                to_str(scene->component_context->get_value(
                                    nodeid)));
                        } else {
                            ImGui::Text("%s",
                                to_str(scene->get_base(nodeid)->get(sock)));
                        }
                    });
                EndTooltip();
            }
        }
        int start_pin_id = 0;
        int end_pin_id   = 0;
        if (ImNodes::IsLinkCreated(&start_pin_id, &end_pin_id)) {
            sockid from_sock = 0, to_sock = 0;
            Node from = decode_pair(start_pin_id, &from_sock);
            Node to   = decode_pair(end_pin_id, &to_sock);
            scene->connect(to, to_sock, from, from_sock);
        };

        int id;
        if (_is_mouse_in() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            if (ImNodes::IsLinkHovered((int*)&id)) {
                menu = MenuType::LINK;
                if (!ImNodes::IsLinkSelected(id)) {
                    ImNodes::SelectLink(id);
                }

            } else if (ImNodes::IsNodeHovered((int*)&id)) {
                if (!ImNodes::IsNodeSelected(id)) {
                    ImNodes::SelectNode(id);
                }
                menu = MenuType::NODE;
            } else {
                menu = MenuType::NEW;
            }
            is_active = true;
            ImGui::OpenPopup("##NodeContextOptions");
        }
        if (is_active
            && ImGui::BeginPopup(
                "##NodeContextOptions", ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGuiCol_PopupBg);
            int len = ImNodes::NumSelectedNodes();
            ImNodes::GetSelectedNodes(nodeids);

            switch (menu) {
            case MenuType::NODE:
                if (IconButton(ICON_LC_TRASH, _("Delete Node"))) {
                    ImNodes::ClearNodeSelection();
                    scene->remove_node(decode_pair(id));
                    ImGui::CloseCurrentPopup();
                }
                if (IconButton(ICON_LC_CLIPBOARD_COPY, _("Copy"))) {
                    is_copied          = true;
                    copy_node_position = ImGui::GetMousePos();
                }
                break;
            case MenuType::LINK:
                if (IconButton(ICON_LC_CIRCLE_SLASH_2, _("Disconnect"))) {
                    scene->disconnect(id);
                    ImGui::CloseCurrentPopup();
                }
                break;
            case MenuType::NEW: break;
            }
            ImGui::BeginDisabled(!is_copied);
            if (IconButton(ICON_LC_CLIPBOARD_PASTE, _("Paste"))) {
                for (int i = 0; i < len; i++) {
                    Node node = decode_pair(nodeids[i]);
                    scene->duplicate_node(node);
                    ImVec2 mouse = ImGui::GetMousePos();
                    mouse        = ImVec2(mouse.x - copy_node_position.x,
                               mouse.y - copy_node_position.y);
                    auto nref    = scene->get_base(node);
                    nref->move(
                        Point { static_cast<int16_t>(nref->point().x + mouse.x),
                            static_cast<int16_t>(nref->point().y + mouse.y) });
                }
            }
            ImGui::EndDisabled();
            if (len > 1) {
                if (IconButton(ICON_LC_TRASH_2, _("Delete Selected"))) {
                    ImNodes::ClearNodeSelection();
                    for (int i = 0; i < len; i++) {
                        scene->remove_node(decode_pair(nodeids[i]));
                    }
                }
            }

            if (ImGui::BeginMenu("Create")) {
                bool created = true;
                Node node;
                if (ImGui::MenuItem(_("Input"))) {
                    node = scene->add_node<Input>();
                } else if (ImGui::MenuItem(_("Output"))) {
                    node = scene->add_node<Output>();
                } else if (ImGui::MenuItem(_("Timer"))) {
                    node = scene->add_node<Input>(static_cast<sockid>(10));
                } else if (ImGui::MenuItem(_("NOT Gate"))) {
                    node = scene->add_node<Gate>(Gate::Type::NOT);
                } else if (ImGui::MenuItem(_("AND Gate"))) {
                    node = scene->add_node<Gate>(Gate::Type::AND);
                } else if (ImGui::MenuItem(_("NAND Gate"))) {
                    node = scene->add_node<Gate>(Gate::Type::NAND);
                } else if (ImGui::MenuItem(_("OR Gate"))) {
                    node = scene->add_node<Gate>(Gate::Type::OR);
                } else if (ImGui::MenuItem(_("NOR Gate"))) {
                    node = scene->add_node<Gate>(Gate::Type::NOR);
                } else if (ImGui::MenuItem(_("XOR Gate"))) {
                    node = scene->add_node<Gate>(Gate::Type::XOR);
                } else if (ImGui::MenuItem(_("XNOR Gate"))) {
                    node = scene->add_node<Gate>(Gate::Type::XNOR);
                } else {
                    created = false;
                }
                if (created) {
                    ImVec2 mouse = ImGui::GetMousePos();
                    ImNodes::SetNodeScreenSpacePos(node.numeric(), mouse);
                    auto pos = ImNodes::GetNodeGridSpacePos(node.numeric());
                    scene->get_base(node)->move(
                        { static_cast<int16_t>(std::floor(pos.x)),
                            static_cast<int16_t>(std::floor(pos.y)) });
                }
                ImGui::EndMenu();
            }

            ImGui::PopStyleColor();
            ImGui::EndPopup();
        }
    } else {
        ImNodes::EndNodeEditor();
    }
}
void Editor::_show_node(ComponentContext& node, uint16_t, bool)
{
    uint32_t compin  = Node { 0, Node::COMPONENT_INPUT }.numeric();
    uint32_t compout = Node { 0, Node::COMPONENT_OUTPUT }.numeric();

    ImNodes::BeginNode(compin);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text(_("Component Input"));
    ImNodes::EndNodeTitleBar();
    for (size_t i = 0; i < node.inputs.size(); i++) {
        ImNodes::BeginOutputAttribute(encode_pair(node.get_input(i), 0, true),
            to_shape(node.inputs[i].size() > 0, true));
        ImGui::Text("%zu", i + 1);
        ImNodes::EndInputAttribute();
    }
    ImNodes::EndNode();

    ImNodes::BeginNode(compout);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text(_("Component Output"));
    ImNodes::EndNodeTitleBar();
    for (size_t i = 0; i < node.outputs.size(); i++) {
        ImNodes::BeginInputAttribute(encode_pair(node.get_output(i), 0, false),
            to_shape(node.outputs[i] != 0, true));
        ImGui::Text("%zu", i + 1);
        ImNodes::EndInputAttribute();
    }
    ImNodes::EndNode();
}

void Editor::_show_node(Input& node, uint16_t id, bool is_changed)
{
    Node nodeinfo   = Node { id, Node::Type::INPUT };
    uint32_t nodeid = nodeinfo.numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node, nodeid, is_changed);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("%s %u", node.is_timer() ? _("Timer") : _("Input"), id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(encode_pair(nodeinfo, 0, true),
        to_shape(node.output.size() > 0, false));
    if (node.is_timer()) {
        ImGui::PushItemWidth(60);
        float freq_value = static_cast<float>(node.freq()) / 10.f;
        if (ImGui::SliderFloat("Hz", &freq_value, 0.1f, 5.0f, "%.1f")) {
            if (freq_value != node.freq()) {
                node.set_freq(freq_value * 10);
            }
        }
        ImGui::PopItemWidth();
    } else {
        ToggleButton(node);
    }
    ImGui::SameLine();
    ImGui::Text("1");
    ImNodes::EndOutputAttribute();

    ImNodes::EndNode();
}

void Editor::_show_node(Output& node, uint16_t id, bool is_changed)
{
    Node nodeinfo   = Node { id, Node::Type::OUTPUT };
    uint32_t nodeid = nodeinfo.numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node, nodeid, is_changed);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text(_("Output %u"), id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(
        encode_pair(nodeinfo, 0, false), to_shape(node.is_connected(), false));
    ImGui::Text("1");
    ImNodes::EndInputAttribute();

    ImNodes::EndNode();
}

void Editor::_show_node(Gate& node, uint16_t id, bool is_changed)
{
    Node nodeinfo   = Node { id, Node::Type::GATE };
    uint32_t nodeid = nodeinfo.numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node, nodeid, is_changed);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text(_("%s Gate %u"), to_str<Gate::Type>(node.type()), id);
    ImNodes::EndNodeTitleBar();

    for (size_t i = 0; i < node.inputs.size(); i++) {
        if (i == node.inputs.size() / 2) {
            ImNodes::BeginOutputAttribute(encode_pair(nodeinfo, 0, true),
                to_shape(node.output.size() > 0, false));
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::CalcTextSize("         ").x);
            ImGui::Text("1");
            ImNodes::EndOutputAttribute();
        }
        ImNodes::BeginInputAttribute(encode_pair(nodeinfo, i, false),
            to_shape(node.is_connected(), true));
        ImGui::Text("%zu", i + 1);
        ImNodes::EndInputAttribute();
    }

    ImNodes::EndNode();
}

void Editor::_show_node(Component& node, uint16_t id, bool is_changed)
{
    Node nodeinfo   = Node { id, Node::Type::COMPONENT };
    uint32_t nodeid = nodeinfo.numeric();
    ImNodes::BeginNode(nodeid);
    _sync_position(node, nodeid, is_changed);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text(_("Component Node %u"), id);
    ImNodes::EndNodeTitleBar();

    for (size_t i = 0; i < node.inputs.size(); i++) {
        if (i == node.inputs.size() / 2) {
            for (size_t j = 0; j < node.outputs.size(); j++) {
                ImNodes::BeginOutputAttribute(encode_pair(nodeinfo, j, true),
                    to_shape(!node.outputs[j].empty(), false));
                ImGui::SetCursorPosX(ImGui::GetCursorPosX()
                    + ImGui::CalcTextSize("         ").x);
                ImGui::Text("%zu", j + 1);
                ImNodes::EndOutputAttribute();
            }
        }
        ImNodes::BeginInputAttribute(encode_pair(nodeinfo, i, false),
            to_shape(node.is_connected(), true));
        ImGui::Text("%zu", i + 1);
        ImNodes::EndInputAttribute();
    }

    ImNodes::EndNode();
}

void Editor::_sync_position(BaseNode& node, uint32_t node_id, bool is_changed)
{
    if (is_changed) {
        ImNodes::SetNodeGridSpacePos(
            node_id, { (float)node.point().x, (float)node.point().y });
    } else {
        auto pos = ImNodes::GetNodeGridSpacePos(node_id);
        pos      = { std::floor(pos.x), std::floor(pos.y) };
        ImNodes::SetNodeGridSpacePos(node_id, pos);
        if (pos.x != node.point().x || pos.y != node.point().y) {
            node.move(
                { static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y) });
        }
    }
}

void Editor::_context_menu_new(Ref<Scene>) { }

} // namespace ic::ui
