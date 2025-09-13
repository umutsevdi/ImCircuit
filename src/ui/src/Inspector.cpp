#include <IconsLucide.h>
#include <imgui.h>
#include <imnodes.h>
#include "components.h"
#include "core.h"
#include "ui.h"

namespace ic::ui {

struct Inspector final : public Window {
    Inspector();
    ~Inspector() = default;

    virtual void show(Ref<Scene>, bool switched) override;
    virtual const char* tooltip(void) override
    {
        return _("A panel that displays and allows you to modify the\n"
                 "attributes of the currently selected logic gate node.");
    }

    int nodeids[1 << 20] = { 0 };
    char buffer[128];

private:
    void _disconnect_button_tooltip();
    void _input_table(Ref<Scene>, const std::vector<relid>&);
    void _output_table(Ref<Scene>, const std::vector<relid>&);
    void _inspector_input(Ref<Scene>, Node);
    void _inspector_output(Ref<Scene>, Node);
    void _inspector_component(Ref<Scene>, Node);
    void _inspector_gate(Ref<Scene>, Node);
    void _inspector_component_context(Ref<Scene>, Node);
    void _inspector_tab(Ref<Scene>, Node);
};
REGISTER_WINDOW("Inspector", Inspector)

void Inspector::show(Ref<Scene> scene, bool)
{
    if (scene != nullptr) {
        int len = ImNodes::NumSelectedNodes();
        ImNodes::GetSelectedNodes(nodeids);
        if (len) {
            ImGui::BeginTabBar("InspectorTabs");
            for (int i = 0; i < len; i++) {
                Node node = decode_pair(nodeids[i]);
                snprintf(buffer, 128, "%s@%u", to_str<Node::Type>(node.type),
                    node.index);
                to_str<Node::Type>(node.type);
                if (ImGui::BeginTabItem(
                        buffer, nullptr, ImGuiTabItemFlags_NoReorder)) {
                    _inspector_tab(scene, node);
                    ImGui::EndTabItem();
                };
            }
            ImGui::EndTabBar();
            if (len > 1) {
                ImGui::PushFont(get_font(FontFlags::ITALIC), FONT_SMALL);
                ImGui::Text(_("%d items selected."), len);
                ImGui::PopFont();
                ImGui::SameLine();
                if (IconButton(ICON_LC_TRASH_2, _("Delete All"))) {
                    ImNodes::ClearNodeSelection();
                    for (int i = 0; i < len; i++) {
                        Node node = decode_pair(nodeids[i]);
                        L_DEBUG("Delete %s@%d", to_str<Node::Type>(node.type),
                            node.index);
                        scene->remove_node(node);
                    }
                }
            }
        }
    }
}

// FIXME Delete node failure
void Inspector::_inspector_tab(Ref<Scene> scene, Node node)
{
    const static float KEY_WIDTH = ImGui::CalcTextSize("SOCKET COUNT").x;
    ImGui::BeginChild(
        "##InspectorFrame", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);
    if (ImGui::BeginTable(
            "##InspectorTable", 2, ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn(
            "##Key", ImGuiTableColumnFlags_WidthFixed, KEY_WIDTH);
        ImGui::NextColumn();
        ImGui::TableSetupColumn("##Value", ImGuiTableColumnFlags_WidthStretch);
        TablePair(Field(_("Id")), ImGui::Text("%u", node.index));
        TablePair(
            Field(_("Type")), ImGui::Text("%s", to_str<Node::Type>(node.type)));

        if (node.type != Node::Type::COMPONENT_INPUT
            && node.type != Node::Type::COMPONENT_OUTPUT) {
            TableKey(Field(_("Position")));
            Point newp = PositionSelector(
                scene->get_base(node)->point(), _("Inspector"));
            if (auto nref = scene->get_base(node); newp != nref->point()) {
                nref->move(newp);
            }
        }

        switch (node.type) {
        case Node::Type::INPUT: _inspector_input(scene, node); break;
        case Node::Type::OUTPUT: _inspector_output(scene, node); break;
        case Node::Type::GATE: _inspector_gate(scene, node); break;
        case Node::Type::COMPONENT: _inspector_component(scene, node); break;
        default: _inspector_component_context(scene, node); break;
        }
    };
    if (IconButton(ICON_LC_EYE, _("Select Node"))) {
        ImNodes::ClearNodeSelection();
        switch (node.type) {
        case Node::Type::COMPONENT_INPUT:
        case Node::Type::COMPONENT_OUTPUT:
            ImNodes::SelectNode(Node { 0, node.type }.numeric());
            break;
        default: ImNodes::SelectNode(node.numeric()); break;
        }
    }
    if (node.type != Node::Type::COMPONENT_OUTPUT
        && node.type != Node::Type::COMPONENT_INPUT) {
        ImGui::SameLine();
        if (IconButton(ICON_LC_TRASH, _("Delete Node"))) {
            ImNodes::ClearNodeSelection();
            scene->remove_node(node);
            return;
        }
    }
    ImGui::EndChild();
}
void Inspector::_inspector_input(Ref<Scene> scene, Node node)
{
    auto _node                = scene->get_node<Input>(node);
    constexpr size_t SIZE     = 20;
    static float values[SIZE] = { 0 };
    static int frame_count    = 0;
    if (_node->is_timer()) {
        TablePair(Field(_("Value")), ImGui::Text("%s", to_str(_node->get())));
        TableKey(Field(_("Frequency")));
        float freq_value = static_cast<float>(_node->freq()) / 10.f;
        if (ImGui::SliderFloat("Hz", &freq_value, 0.1f, 5.0f, "%.1f")) {
            if (freq_value != _node->freq()) {
                _node->set_freq(freq_value * 10);
            }
        }
        if (++frame_count % SIZE == 0) {
            for (size_t i = 1; i < SIZE; i++) {
                values[i - 1] = values[i];
            }
        }
        values[SIZE - 1] = _node->get() == State::TRUE;
        ImGui::PlotLines("##Frequency", values, SIZE, 0, nullptr, 0.0f, 1.0f);
    } else {
        TablePair(Field(_("Value")), ToggleButton(*_node));
    }

    TableKey(Field(_("Outputs")));
    if (ImGui::BeginTable("InputList", 3,
            ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn(_("Socket"), ImGuiTableColumnFlags_WidthFixed);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(
            _("Connection"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(_("Value"), ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        TableKey(Field("1"));
        _output_table(scene, _node->output);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%s", to_str(_node->get()));
        ImGui::EndTable();
    }
    ImGui::EndTable();
}

void Inspector::_inspector_output(Ref<Scene> scene, Node node)
{
    auto _node = scene->get_node<Output>(node);
    TablePair(Field("Value"), ImGui::Text("%s", to_str(_node->get())));
    std::vector<relid> in;
    in.push_back(_node->input);
    TablePair(Field(_("Inputs")), _input_table(scene, in));
    ImGui::EndTable();
}

void Inspector::_inspector_gate(Ref<Scene> scene, Node node)
{
    const static ImVec2 __selector_size = ImGui::CalcTextSize("-000000000000");

    auto _node = scene->get_node<Gate>(node);
    TablePair(Field(_("Value")), ImGui::Text("%s", to_str(_node->get())));
    TablePair(Field(_("Gate Type")),
        ImGui::Text("%s", to_str<Gate::Type>(_node->type())));
    ImGui::BeginDisabled(_node->type() == Gate::Type::NOT);
    TableKey(Field(_("Socket Count")));
    size_t socket_count = _node->inputs.size();
    size_t inc          = 1;
    ImGui::PushItemWidth(__selector_size.x);
    if (ImGui::InputScalar("##SocketCount", ImGuiDataType_U8, &socket_count,
            &inc, &inc, nullptr)) {
        bool keep = true;
        while (socket_count != _node->inputs.size() && keep) {
            if (socket_count > _node->inputs.size()) {
                keep = _node->increment();
            } else {
                keep = _node->decrement();
            }
        }
    }
    ImGui::EndDisabled();
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    TablePair(Field(_("Inputs")), _input_table(scene, _node->inputs));

    TableKey(Field(_("Outputs")));
    if (ImGui::BeginTable("InputList", 3,
            ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn(_("Socket"), ImGuiTableColumnFlags_WidthFixed);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(
            _("Connection"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(_("Value"), ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        TablePair(Field("1"), _output_table(scene, _node->output));
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%s", to_str(_node->get()));
        ImGui::EndTable();
    }
    ImGui::EndTable();
}

void Inspector::_inspector_component(Ref<Scene> scene, Node node)
{
    auto _node = scene->get_node<Component>(node);
    TableKey(Field(_("Value")));
    ImGui::Text("(");
    ImGui::SameLine();
    for (size_t i = 0; i < _node->outputs.size(); i++) {
        ImGui::Text("%s", to_str(_node->get(i)));
        ImGui::SameLine();
    }
    ImGui::Text(")");
    TablePair(Field(_("Inputs")), _input_table(scene, _node->inputs));
    TableKey(Field(_("Outputs")));
    if (ImGui::BeginTable("InputList", 3,
            ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn(_("Socket"), ImGuiTableColumnFlags_WidthFixed);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(
            "Connection", ImGuiTableColumnFlags_WidthStretch);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(_("Value"), ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        for (auto& out : _node->outputs) {
            TablePair(Field("%d", out.first), _output_table(scene, out.second));
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", to_str(_node->get(out.first)));
        }

        ImGui::EndTable();
    }
    ImGui::EndTable();
}

void Inspector::_inspector_component_context(Ref<Scene> scene, Node node)
{
    ComponentContext& ctx = scene->component_context.value();
    if (node.type == Node::Type::COMPONENT_INPUT) {
        TableKey(Field(_("Outputs")));
        if (ImGui::BeginTable("InputsComponentInputList", 3,
                ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn(
                _("Socket"), ImGuiTableColumnFlags_WidthFixed);
            ImGui::NextColumn();
            ImGui::TableSetupColumn(
                _("Connection"), ImGuiTableColumnFlags_WidthStretch);
            ImGui::NextColumn();
            ImGui::TableSetupColumn(
                _("Value"), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            for (size_t i = 0; i < ctx.inputs.size(); i++) {
                TablePair(
                    Field("%zu", i + 1), _output_table(scene, ctx.inputs[i]));
                ImGui::TableSetColumnIndex(2);
                State value = ctx.get_value(ctx.get_input(i));
                ImGui::PushID((std::to_string(i) + "btn").c_str());
                // TODO
                // if (State new_value = ToggleButton(value, true);
                //     value != new_value) {
                //     ctx.set_value(ctx.get_input(i), new_value);
                // }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    } else {
        TablePair(Field(_("Inputs")),
            _input_table(scene, scene->component_context->outputs));
    }
    ImGui::EndTable();
}

void Inspector::_input_table(Ref<Scene> scene, const std::vector<relid>& inputs)
{
    if (ImGui::BeginTable("Inputs", 4,
            ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn(_("Socket"), ImGuiTableColumnFlags_WidthFixed);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(
            _("Connection"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(
            "##Disconnect", ImGuiTableColumnFlags_WidthFixed);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(_("Value"), ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < inputs.size(); i++) {
            TableKey(Field("%zu", i + 1));
            ImGui::SameLine();
            ImGui::PushFont(get_font(FontFlags::BOLD), 0.f);
            State value = State::DISABLED;
            if (inputs[i] != 0) {
                Ref<Rel> r = scene->get_rel(inputs[i]);
                NodeTypeTitle(r->from_node, r->from_sock);
                value = r->value;
            }
            ImGui::TableSetColumnIndex(2);
            ImGui::BeginDisabled(inputs[i] == 0);
            ImGui::PushID(std::to_string(i).c_str());
            if (IconButton(ICON_LC_CIRCLE_SLASH_2, "")) {
                scene->disconnect(inputs[i]);
            }
            ImGui::PopID();
            ImGui::EndDisabled();
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", to_str(value));
            ImGui::PopFont();
        }
        if (ImGui::TableGetHoveredColumn() == 2) {
            _disconnect_button_tooltip();
        };
        ImGui::EndTable();
    };
}

void Inspector::_output_table(
    Ref<Scene> scene, const std::vector<relid>& outputs)
{
    if (ImGui::BeginTable("Outputs", 2,
            ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg)) {
        ImGui::TableHeader("Outputs");
        ImGui::TableSetupColumn(
            _("Connection"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(
            _("Disconnect"), ImGuiTableColumnFlags_WidthFixed);
        for (size_t i = 0; i < outputs.size(); i++) {
            ImGui ::TableNextRow();
            ImGui ::TableSetColumnIndex(0);
            ImGui::PushFont(get_font(FontFlags::BOLD), 0.f);
            if (outputs[i] == 0) {
                ImGui::TextColored(get_active_style().black_bright, "%s",
                    to_str<State>(State::DISABLED));
            } else {
                Ref<Rel> r = scene->get_rel(outputs[i]);
                NodeTypeTitle(r->to_node, r->to_sock);
            }
            ImGui::PopFont();
            ImGui ::TableSetColumnIndex(1);

            ImGui::BeginDisabled(outputs[i] == 0);
            ImGui::PushID(("output_" + std::to_string(i)).c_str());
            if (IconButton(ICON_LC_CIRCLE_SLASH_2, "")) {
                scene->disconnect(outputs[i]);
            }
            ImGui::PopID();
            ImGui::EndDisabled();
        }
        if (ImGui::TableGetHoveredColumn() == 1) {
            _disconnect_button_tooltip();
        };
        ImGui::EndTable();
    }
}

void Inspector::_disconnect_button_tooltip()
{
    if (ImGui::BeginTooltip()) {
        ImGui::Text(_("Disconnect relation"));
        ImGui::EndTooltip();
    }
}
} // namespace ic::ui
