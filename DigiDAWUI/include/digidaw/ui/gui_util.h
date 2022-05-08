#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <cmath>
#include <algorithm>

namespace DigiDAW::UI::Util
{
    struct AudioMeterStyle
    {
        // Meter Range Colors
        ImVec4 lowRangeColor = ImVec4(0.4f, 0.75f, 0.4f, 1.0f);
        ImVec4 midRangeColor = ImVec4(0.75f, 0.8f, 0.1f, 1.0f);
        ImVec4 highRangeColor = ImVec4(0.8f, 0.5f, 0.1f, 1.0f);

        // Dividing Line
        float lineAlpha = 0.3f;

        // Clip Indicator
        ImVec4 activeClipColor = ImVec4(0.9f, 0.3f, 0.3f, 1.0f);

        bool segmented = true;
        int lineSegments = 64;

        int stereoMeterSpacing = 3;
    };

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

    void DrawAudioMeter(
        float avgFraction, float peakFraction, 
        bool clip = false, 
        const AudioMeterStyle& audioMeterStyle = AudioMeterStyle(),
        const ImVec2& sizeArg = ImVec2(0, 0));
    void DrawAudioMeterStereo(
        float leftAvgFraction, float rightAvgFraction, 
        float leftPeakFraction, float rightPeakFraction, 
        bool leftClip = false, bool rightClip = false, 
        const AudioMeterStyle& audioMeterStyle = AudioMeterStyle(),
        const ImVec2& sizeArg = ImVec2(0, 0));
}
