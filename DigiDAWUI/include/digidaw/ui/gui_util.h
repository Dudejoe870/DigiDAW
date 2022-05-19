#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <digidaw/core/audio/engine.h>

#include <cmath>
#include <algorithm>

#include <fmt/core.h>

namespace DigiDAW::UI
{
    class Util
    {
    public:
        struct AudioMeterStyle
        {
            // Meter Range Colors
            ImVec4 lowRangeColor = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
            ImVec4 midRangeColor = ImVec4(0.9f, 1.0f, 0.1f, 1.0f);
            ImVec4 highRangeColor = ImVec4(1.0f, 0.6f, 0.1f, 1.0f);

            // Dividing Line
            float lineAlpha = 0.132f;

            // Clip Indicator
            ImVec4 activeClipColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);

            int meterWidth = 12;

            bool segmented = true;
            bool rounded = true;

            int lineSegments = 64;

            int stereoMeterSpacing = 3;
        };

        static float DecibelToPercentage(float decibel, float minimumDecibel = -60.0f, float maximumDecibel = 0.0f)
        {
            const float absMin = std::abs(minimumDecibel);
            return (std::clamp(decibel, minimumDecibel, maximumDecibel) + absMin) / absMin;
        }

        static void DrawRectRangeV(ImDrawList* drawList, const ImRect& rect, float fraction, ImU32 color, float fractionOffset = 0.0f, float rounding = 0.0f, ImDrawFlags drawFlags = 0)
        {
            ImVec2 p0 = ImVec2(rect.Min.x, ImLerp(rect.Max.y, rect.Min.y, fraction));
            ImVec2 p1 = ImVec2(rect.Max.x, ImLerp(rect.Max.y, rect.Min.y, fractionOffset));
            drawList->AddRectFilled(p0, p1, color, rounding, drawFlags);
        }

        static void Center(float width = 0.0f)
        {
            float windowWidth = ImGui::GetWindowSize().x;

            ImGui::SetCursorPosX((windowWidth - width) * 0.5f);
        }

        static void TextCentered(const char* text)
        {
            float textWidth = ImGui::CalcTextSize(text).x;

            Center(textWidth);
            ImGui::TextUnformatted(text);
        }

        static void TextRightAlign(const char* text, float padding = 0.0f)
        {
            float windowWidth = ImGui::GetWindowSize().x;
            float textWidth = ImGui::CalcTextSize(text).x;

            ImGui::SetCursorPosX((windowWidth - textWidth) - padding);
            ImGui::TextUnformatted(text);
        }

        static float GetMeterLabelWidth(float minimumDecibel = -60.0f)
        {
            return ImGui::CalcTextSize(fmt::format("{}dB", minimumDecibel).c_str()).x;
        }

        static const float audioMeterClipIndicatorHeight;
        static const float audioMeterHeight;
        static const float audioMeterFullHeight;

        static const float channelStripWidth;

        static void DrawMeterLabels(float minimumDecibel = -60.0f);
        static void DrawAudioMeterEx(
            float rmsFraction, float peakFraction,
            bool clip = false,
            const AudioMeterStyle& audioMeterStyle = AudioMeterStyle());
        static void DrawAudioMeter(const std::string& layoutName,
            float rmsFraction, float peakFraction,
            bool clip = false,
            const AudioMeterStyle& audioMeterStyle = AudioMeterStyle());
        static void DrawAudioMeterStereo(const std::string& layoutName,
            float leftRmsFraction, float rightRmsFraction,
            float leftPeakFraction, float rightPeakFraction,
            bool leftClip = false, bool rightClip = false,
            const AudioMeterStyle& audioMeterStyle = AudioMeterStyle());

        static void DrawChannelStripBackground(bool even);
        static float DrawFaderMeterCombo(const std::string& layoutName,
            std::shared_ptr<Core::Audio::Engine>& audioEngine, 
            std::shared_ptr<Core::Audio::TrackState::Mixable> mixable,
            const AudioMeterStyle& audioMeterStyle = AudioMeterStyle(), 
            float maximumDecibel = 6.0f);
        static void DrawMixableControls(const std::string& namePlaceholder,
            std::shared_ptr<Core::Audio::Engine>& audioEngine,
            std::shared_ptr<Core::Audio::TrackState::Mixable> mixable,
            const AudioMeterStyle& audioMeterStyle = AudioMeterStyle(),
            float maximumDecibel = 6.0f);
    };
}
