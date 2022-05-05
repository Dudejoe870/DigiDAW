#include <iostream>
#include <memory>

#include <digidaw/core/audio/engine.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "digidaw/ui/ui.h"
#include "digidaw/ui/timer.h"
#include "res/resources.h"

using namespace DigiDAW;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main()
{
	auto audioEngine = std::make_shared<Core::Audio::Engine>(RtAudio::Api::UNSPECIFIED);

    Core::Audio::TrackState::Bus& mainBus = audioEngine->trackState.AddBus(
        Core::Audio::TrackState::Bus("Main Bus",
            Core::Audio::TrackState::ChannelNumber::Stereo,
			0.0f, 0.0f,
			std::vector<std::vector<unsigned int>>
			{
				std::vector<unsigned int> { 0 },
				std::vector<unsigned int> { 1 }
			})); // Make a Bus that outputs to the first two channels of the output device.
    /*
	audioEngine->trackState.AddTrack(
        Core::Audio::TrackState::Track("Track 1",
            Core::Audio::TrackState::ChannelNumber::Stereo,
			-25.0f, 0.0f,
			std::vector<Core::Audio::TrackState::BusOutput>
			{
				Core::Audio::TrackState::BusOutput(mainBus, std::vector<std::vector<unsigned int>>
				{
					std::vector<unsigned int> { 0 },
                    std::vector<unsigned int> { 1 }
				})
			})); // Make a Stereo Track that outputs both its channels to the L and R channels of the mainBus.
    */
    audioEngine->trackState.AddTrack(
        Core::Audio::TrackState::Track("Track 1",
            Core::Audio::TrackState::ChannelNumber::Mono,
            -25.0f, 0.0f,
            std::vector<Core::Audio::TrackState::BusOutput>
            {
                Core::Audio::TrackState::BusOutput(mainBus, std::vector<std::vector<unsigned int>>
                {
                    std::vector<unsigned int> { 0, 1 }
                })
            })); // Make a Mono Track that outputs both its channels to the L and R channels of the mainBus.

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "DigiDAW", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    gladLoadGL();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    UI::UI ui(audioEngine);

    // Main loop
    while (!glfwWindowShouldClose(window) && !ui.ShouldExit())
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        UI::Timer::UpdatePendingTimers();

        // Render the UI.
        ui.Render();

        // Rendering
        ImGui::Render();
        int displayW, displayH;
        glfwGetFramebufferSize(window, &displayW, &displayH);

        ImVec4 clearColor = ui.GetClearColor();
        glViewport(0, 0, displayW, displayH);
        glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}
