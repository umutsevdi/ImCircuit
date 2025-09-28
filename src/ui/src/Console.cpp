#include <IconsLucide.h>
#include <imgui.h>
#include <string_view>
#include "common.h"
#include "components.h"
#include "ui.h"
namespace ic::ui {

struct Console final : public Window {
    Console();
    ~Console() = default;

    virtual void show(Ref<Scene>, bool switched) override;
    virtual const char* tooltip(void) override
    {
        return _("Displays warning messages.");
    }

private:
    bool auto_scroll = true;

    ImVec4 _log_color(const Theme& style, Message::Severity level)
    {
        switch (level) {
        case Message::DEBUG: return style.blue;
        case Message::INFO: return style.green;
        case Message::WARN: return style.magenta;
        case Message::FATAL:;
        case Message::ERROR: return style.red;
        default: return style.fg;
        }
    };
};
REGISTER_WINDOW("Console", Console);

void Console::show(Ref<Scene>, bool)
{
    const Theme& style = get_active_style();
    if (IconButton(ICON_LC_TRASH, _("Clear"))) {
        ic::fs::clear_log();
    }
    HINT(nullptr, _("Clear"), _("Clears all log messages."));
    ImGui::SameLine();
    ImGui::Checkbox(_("Auto-scroll"), &auto_scroll);
    HINT(nullptr, _("Auto-scroll"),
        _("When enabled, the scrollbar will be locked to the bottom \n"
          "to display live updates."));
    if (ImGui::BeginTable("##ConsoleTable", 4,
            ImGuiTableFlags_Reorderable | ImGuiTableFlags_BordersInner
                | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY
                | ImGuiTableColumnFlags_NoResize)) {
        ImGui::TableHeader("##Console");
        ImGui::TableSetupColumn(_("Time"), ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(
            _("Severity"), ImGuiTableColumnFlags_WidthFixed);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(_("Module"), ImGuiTableColumnFlags_WidthFixed);
        ImGui::NextColumn();
        ImGui::TableSetupColumn(
            _("Message"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        ic::fs::logs_for_each([this, &style](size_t idx, const Message& l) {
            bool selected = false;
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Selectable(("##" + std::to_string(idx)).c_str(), &selected,
                ImGuiSelectableFlags_SpanAllColumns);
            ImGui::SameLine();
            ImGui::TextUnformatted(l.time_str.data());

            ImGui::TableSetColumnIndex(1);
            ImGui::TextColored(
                _log_color(style, l.severity), "%s", l.log_level.data());
            if (!std::string_view { l.module.data() }.empty()) {
                ImGui::TableSetColumnIndex(2);
                ImGui::PushID(idx);
                ImGui::TextColored(style.yellow, "%s", l.module.data());
                ImGui::PopID();
            }
            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(l.expr.data());

            if (selected) {
                std::stringstream buffer {};
                buffer << l.log_level.data() << '\t' << l.file_line.data()
                       << '\t' << l.module.data() << '\t' << l.expr.data()
                       << std::endl;
                Toast(ICON_LC_CLIPBOARD_COPY, _("Clipboard"),
                    _("Message is copied to the clipboard."));
                ImGui::SetClipboardText(buffer.str().c_str());
            }
        });
        if (auto_scroll) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndTable();
    }
}
} // namespace ic::ui
