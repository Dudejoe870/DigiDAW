#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "digidaw/ui/gui_util.h"

namespace DigiDAW::UI
{
    // From https://stackoverflow.com/a/67855985
    void Util::TextCentered(const char* text)
    {
        float windowWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize(text).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextUnformatted(text);
    }

    void Util::TextRightAlign(const char* text, float padding)
    {
        float windowWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize(text).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) - padding);
        ImGui::TextUnformatted(text);
    }

    void Util::DrawAudioMeter(float fraction, const ImVec2& sizeArg)
    {
        const ImU32 greenColor = 
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
        const float greenMaxAmp = 0.50f;
        const ImU32 yellowColor =
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.9f, 0.8f, 0.1f, 1.0f));
        const float yellowMaxAmp = 0.7f;
        const ImU32 orangeColor =
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.9f, 0.6f, 0.1f, 1.0f));
        const ImU32 lineColor =
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.2f));

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = ImGui::CalcItemSize(sizeArg, 12.0f, 320.0f);
        ImRect bb(pos, pos + size);
        ImGui::ItemSize(size, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, 0))
            return;
        
        // Draw meter
        fraction = ImSaturate(fraction);
        ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_Button), false, 0.0f);
        DrawRectRangeV(window->DrawList, bb, 
            std::clamp(fraction, 0.0f, greenMaxAmp), greenColor); // Draw Green section
        if (fraction > greenMaxAmp)
            DrawRectRangeV(window->DrawList, bb, 
                std::clamp(fraction, greenMaxAmp, yellowMaxAmp), yellowColor, greenMaxAmp); // Draw Yellow section
        if (fraction > yellowMaxAmp)
            DrawRectRangeV(window->DrawList, bb, 
                std::clamp(fraction, yellowMaxAmp, 1.0f), orangeColor, yellowMaxAmp); // Draw Orange section
        
        // Draw dividing lines
        const unsigned int lines = 64;
        float lineStep = (bb.Max.y - bb.Min.y) / lines;
        for (unsigned int i = 1; i < lines; ++i)
        {
            float lineY = bb.Min.y + (i * lineStep);
            window->DrawList->AddLine(ImVec2(bb.Min.x, lineY), ImVec2(bb.Max.x, lineY), lineColor);
        }
    }

    void Util::DrawAudioMeterStereo(float leftFraction, float rightFraction, const ImVec2& sizeArg)
    {
        const float stereoMeterSpacing = 3.0f;
        DrawAudioMeter(leftFraction, sizeArg);
        ImGui::SameLine(0.0f, stereoMeterSpacing);
        DrawAudioMeter(rightFraction, sizeArg);
    }
}
