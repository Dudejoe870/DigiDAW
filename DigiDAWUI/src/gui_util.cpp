#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "imgui_stacklayout.h"

#include "digidaw/ui/gui_util.h"

#include "imgui-knobs.h"

#include <fmt/core.h>

namespace DigiDAW::UI
{
    const float Util::audioMeterClipIndicatorHeight = 6.0f;
    const float Util::audioMeterHeight = 220.0f;
    const float Util::audioMeterFullHeight = audioMeterHeight + audioMeterClipIndicatorHeight;

    const float Util::channelStripWidth = 140.0f;

    void Util::DrawMeterLabels(float minimumDecibel)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = ImVec2(GetMeterLabelWidth(minimumDecibel), audioMeterHeight);
        ImRect bb(pos, pos + size);
        ImGui::ItemSize(size);
        if (!ImGui::ItemAdd(bb, 0))
            return;

        for (float i = minimumDecibel + 5; i <= 0; i += 5)
        {
            float percentage = DecibelToPercentage(i, minimumDecibel);
            float y = audioMeterHeight * percentage;
            window->DrawList->AddText(ImVec2(bb.Min.x, bb.Max.y - y), 
                ImGui::GetColorU32(ImGuiCol_Text), 
                fmt::format("{}dB", i).c_str());
        }
    }

    void Util::DrawAudioMeterEx(
        float rmsFraction, float peakFraction,
        bool clip, 
        const AudioMeterStyle& audioMeterStyle)
    {
        const float maxLowRange = 0.5f;
        const float maxMidRange = 0.7f;
        const float deactiveClipAlpha = 0.2f;

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        ImVec2 pos = window->DC.CursorPos + ImVec2(0, audioMeterClipIndicatorHeight);
        ImVec2 size = ImVec2(audioMeterStyle.meterWidth, audioMeterHeight);
        ImRect bb(pos, pos + size);
        ImRect meterSize(bb);
        bb.Min.y -= audioMeterClipIndicatorHeight; // Add clip indicator to overall bounding box
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

    void Util::DrawAudioMeter(const std::string& layoutName,
        float rmsFraction, float peakFraction,
        bool clip,
        const AudioMeterStyle& audioMeterStyle)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        {
            ImGui::BeginHorizontal(layoutName.c_str());
            {
                DrawAudioMeterEx(
                    rmsFraction, peakFraction,
                    clip,
                    audioMeterStyle);
                ImGui::Dummy(ImVec2(3.0f, 0.0f));
                DrawMeterLabels();
            }
            ImGui::EndHorizontal();
        }
        ImGui::PopStyleVar();
    }

    void Util::DrawAudioMeterStereo(const std::string& layoutName,
        float leftRmsFraction, float rightRmsFraction,
        float leftPeakFraction, float rightPeakFraction, 
        bool leftClip, bool rightClip, 
        const AudioMeterStyle& audioMeterStyle)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        {
            ImGui::BeginHorizontal(layoutName.c_str(), ImVec2(0.0f, 0.0f), 0.0f);
            {
                DrawAudioMeterEx(
                    leftRmsFraction, leftPeakFraction,
                    leftClip,
                    audioMeterStyle);
                ImGui::Dummy(ImVec2(audioMeterStyle.stereoMeterSpacing, 0.0f));
                DrawAudioMeterEx(
                    rightRmsFraction, rightPeakFraction,
                    rightClip,
                    audioMeterStyle);
                ImGui::Dummy(ImVec2(3.0f, 0.0f));
                DrawMeterLabels();
            }
            ImGui::EndHorizontal();
        }
        ImGui::PopStyleVar();
    }

    void Util::DrawChannelStripBackground(bool even)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        ImGuiViewport* viewport = ImGui::GetWindowViewport();

        ImVec2 pos = window->DC.CursorPos - ImVec2(0.0f, style.WindowPadding.y);
        ImVec2 size = ImVec2(channelStripWidth, viewport->Size.y);
        ImRect bb(pos, pos + size);

        ImU32 bgColor = even ? ImGui::GetColorU32(ImGuiCol_TableRowBg) : ImGui::GetColorU32(ImGuiCol_TableRowBgAlt);
        ImGui::RenderFrame(bb.Min, bb.Max, bgColor, false);
    }

    float Util::DrawFaderMeterCombo(const std::string& layoutName,
        std::shared_ptr<Core::Audio::Engine>& audioEngine, 
        std::shared_ptr<Core::Audio::TrackState::Mixable> mixable,
        const AudioMeterStyle& audioMeterStyle, 
        float maximumDecibel)
    {
        float faderGainLinear = std::powf(10.0f, mixable->gain / 20.0f);
        ImGui::BeginHorizontal(layoutName.c_str(), ImVec2(0.0f, 0.0f), 0.5f);
        {
            const float maxSlider = std::powf(10.0f, maximumDecibel / 20.0f);

            ImGui::VSliderFloat("##gain", ImVec2(20.0f, audioMeterFullHeight), &faderGainLinear, 0.0f, maxSlider, "",
                ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_NoRoundToFormat);

            const Core::Audio::Mixer::MixableInfo& mixableInfo = audioEngine->mixer.GetMixableInfo(mixable);
            if (mixableInfo.channels.size() == 1)
            {
                DrawAudioMeter("##audio_meter_layout",
                    DecibelToPercentage(mixableInfo.channels[0].rms, audioEngine->mixer.minimumDecibelLevel),
                    DecibelToPercentage(mixableInfo.channels[0].peak, audioEngine->mixer.minimumDecibelLevel),
                        false, audioMeterStyle);
            }
            else if (mixableInfo.channels.size() == 2)
            {
                DrawAudioMeterStereo("##audio_meter_layout",
                    DecibelToPercentage(mixableInfo.channels[0].rms, audioEngine->mixer.minimumDecibelLevel),
                    DecibelToPercentage(mixableInfo.channels[1].rms, audioEngine->mixer.minimumDecibelLevel),
                    DecibelToPercentage(mixableInfo.channels[0].peak, audioEngine->mixer.minimumDecibelLevel),
                    DecibelToPercentage(mixableInfo.channels[1].peak, audioEngine->mixer.minimumDecibelLevel),
                        false, false, audioMeterStyle);
            }
        }
        ImGui::EndHorizontal();
        return faderGainLinear;
    }

    void Util::DrawMixableControls(const std::string& namePlaceholder,
        std::shared_ptr<Core::Audio::Engine>& audioEngine,
        std::shared_ptr<Core::Audio::TrackState::Mixable> mixable,
        const AudioMeterStyle& audioMeterStyle,
        float maximumDecibel)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 3.0f));
        {
            ImGui::InputTextEx("##name", "Track Name", mixable->name, 256, ImVec2(channelStripWidth * 0.71f, 0.0f), 0);

            ImGuiKnobs::Knob("Pan", &mixable->pan, -100.0f, 100.0f, 0.0f, "%.0f",
                ImGuiKnobVariant_Wiper, 48.0f, ImGuiKnobFlags_DragHorizontal);

            float faderGainLinear = DrawFaderMeterCombo(
                "##fader_meter_layout", audioEngine, mixable, audioMeterStyle, 6.0f);

            mixable->gain = 20.0f * std::log10f(faderGainLinear); // Convert from a linear value to decibels
            ImGui::SetNextItemWidth(64.0f);
            ImGui::InputFloat("##gain_input", &mixable->gain, 0.0f, 0.0f, "%.1fdB");
        }
        ImGui::PopStyleVar();
    }
}
