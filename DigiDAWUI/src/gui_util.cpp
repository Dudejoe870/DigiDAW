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

    void Util::DrawAudioMeter(
        float rmsFraction, float peakFraction,
        bool clip, 
        const AudioMeterStyle& audioMeterStyle)
    {
        const float clipIndicatorHeight = 6.0f;
        const float maxLowRange = 0.5f;
        const float maxMidRange = 0.7f;
        const float deactiveClipAlpha = 0.2f;

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        ImVec2 pos = window->DC.CursorPos + ImVec2(0, clipIndicatorHeight);
        ImVec2 size = ImVec2(audioMeterStyle.meterWidth, 320.0f);
        ImRect bb(pos, pos + size);
        ImRect meterSize(bb);
        bb.Min.y -= clipIndicatorHeight; // Add clip indicator to overall bounding box
        ImGui::ItemSize(size, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, 0))
            return;

        float lineStep = (meterSize.Max.y - meterSize.Min.y) / audioMeterStyle.lineSegments;

        float lowRangeMaxFrac = maxLowRange;
        float midRangeMaxFrac = maxMidRange;
        if (audioMeterStyle.segmented)
        {
            float lineStepPercentage = lineStep / (meterSize.Max.y - meterSize.Min.y);
            lowRangeMaxFrac = std::roundf(maxLowRange / lineStepPercentage) * lineStepPercentage;
            midRangeMaxFrac = std::roundf(maxMidRange / lineStepPercentage) * lineStepPercentage;
        }

        float rounding = audioMeterStyle.rounded ? style.FrameRounding : 0.0f;

        // Draw frame
        ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, rounding);
        bb.Expand(ImVec2(-style.FrameBorderSize, -style.FrameBorderSize));
        meterSize.Expand(ImVec2(-style.FrameBorderSize, -style.FrameBorderSize));

        // TODO: LUFS meter
        // Draw RMS meter
        rmsFraction = ImSaturate(rmsFraction);
        if (rmsFraction > 0.0f)
        {
            DrawRectRangeV(window->DrawList, meterSize,
                std::clamp(rmsFraction, 0.0f, lowRangeMaxFrac),
                ImGui::ColorConvertFloat4ToU32(audioMeterStyle.lowRangeColor), 0.0f, 
                    rounding, ImDrawFlags_RoundCornersBottom); // Draw low range section
            if (rmsFraction > lowRangeMaxFrac)
                DrawRectRangeV(window->DrawList, meterSize,
                    std::clamp(rmsFraction, lowRangeMaxFrac, midRangeMaxFrac),
                    ImGui::ColorConvertFloat4ToU32(audioMeterStyle.midRangeColor), lowRangeMaxFrac, 0.0f); // Draw mid range section
            if (rmsFraction > midRangeMaxFrac)
                DrawRectRangeV(window->DrawList, meterSize,
                    std::clamp(rmsFraction, midRangeMaxFrac, 1.0f),
                    ImGui::ColorConvertFloat4ToU32(audioMeterStyle.highRangeColor), midRangeMaxFrac, 0.0f); // Draw high range section
        }
        
        // Draw dividing lines
        if (audioMeterStyle.segmented)
        {
            for (unsigned int i = 1; i < audioMeterStyle.lineSegments; ++i)
            {
                float lineY = meterSize.Min.y + (i * lineStep);
                window->DrawList->AddLine(ImVec2(meterSize.Min.x, lineY), ImVec2(meterSize.Max.x, lineY),
                    ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, audioMeterStyle.lineAlpha)), 0.5f);
            }
        }

        // Draw Peak meter line
        if (peakFraction > 0.0f)
        {
            peakFraction = ImSaturate(peakFraction);
            ImU32 peakMeterColor = ImGui::ColorConvertFloat4ToU32(audioMeterStyle.lowRangeColor);
            if (peakFraction > lowRangeMaxFrac) 
                peakMeterColor = ImGui::ColorConvertFloat4ToU32(audioMeterStyle.midRangeColor);
            else if (peakFraction > midRangeMaxFrac) 
                peakMeterColor = ImGui::ColorConvertFloat4ToU32(audioMeterStyle.highRangeColor);
            float peakHeight = ImLerp(meterSize.Max.y, meterSize.Min.y, peakFraction);
            window->DrawList->AddLine(
                ImVec2(meterSize.Min.x, peakHeight),
                ImVec2(meterSize.Max.x, peakHeight), peakMeterColor, 1.5f);
        }

        // Draw clip indicator
        ImU32 clipIndicatorColor = clip ? 
            ImGui::ColorConvertFloat4ToU32(audioMeterStyle.activeClipColor) : 
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, deactiveClipAlpha));
        window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Max.x, meterSize.Min.y), clipIndicatorColor, 
            rounding, ImDrawFlags_RoundCornersTop);
    }

    void Util::DrawAudioMeterStereo(
        float leftRmsFraction, float rightRmsFraction,
        float leftPeakFraction, float rightPeakFraction, 
        bool leftClip, bool rightClip, 
        const AudioMeterStyle& audioMeterStyle)
    {
        DrawAudioMeter(
            leftRmsFraction, leftPeakFraction,
            leftClip, 
            audioMeterStyle);
        ImGui::SameLine(0.0f, audioMeterStyle.stereoMeterSpacing);
        DrawAudioMeter(
            rightRmsFraction, rightPeakFraction,
            rightClip, 
            audioMeterStyle);
    }
}
