#include "gui_util.h"

#include "imgui.h"

namespace DigiDAW::UI
{
    // From https://stackoverflow.com/a/67855985
    void Util::TextCentered(const char* text)
    {
        float windowWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize(text).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text(text);
    }
}
