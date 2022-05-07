#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <cmath>
#include <algorithm>

namespace DigiDAW::UI::Util
{
    inline float AmplitudeToDecibelPercentage(float amplitude, float minimumDecibel = -60.0f, float maximumDecibel = 0.0f)
    {
        const float absMin = std::abs(minimumDecibel);
        return (std::clamp(20.0f * std::log10f(amplitude), minimumDecibel, maximumDecibel) + absMin) / absMin;
    }

    inline void DrawRectRangeV(ImDrawList* drawList, const ImRect& rect, float fraction, ImU32 color, float fractionOffset = 0.0f)
    {
        ImVec2 p0 = ImVec2(rect.Min.x, ImLerp(rect.Max.y, rect.Min.y, fractionOffset));
        ImVec2 p1 = ImVec2(rect.Max.x, ImLerp(rect.Max.y, rect.Min.y, fraction));
        drawList->AddRectFilled(p0, p1, color, 0.0f);
    }

    void TextCentered(const char* text);
    void TextRightAlign(const char* text, float padding = 0.0f);

    void DrawAudioMeter(float fraction, const ImVec2& sizeArg = ImVec2(0, 0));
    void DrawAudioMeterStereo(float leftFraction, float rightFraction, const ImVec2& sizeArg = ImVec2(0, 0));
}
