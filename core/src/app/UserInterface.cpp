#include <memory>
#include <vector>
#include <string>
#include <cmath>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>

#include <App.hpp>
#include <AppContext.hpp>

namespace UserInterface {

    static bool showTooltipIcons = false;
    static int selectedColorRampIndex = 0;

    void SetCustomTheme() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;
        ImGui::StyleColorsDark();

        // ROUNDING
        style.WindowRounding = 4.0f;
        style.FrameRounding = 4.0f;
        style.ChildRounding = 6.0f;
        style.PopupRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.ScrollbarRounding = 6.0f;

        // WINDOW
        style.WindowBorderSize = 0.0f;
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);    // WINDOW TITLE CENTERED
        
        style.ItemSpacing = ImVec2(8.0f, 6.0f);
        style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
        style.ScrollbarSize = 18.0f;
        style.FramePadding = ImVec2(10.0f, 6.0f);
        style.FrameBorderSize = 0.0f;
        style.GrabMinSize = 10.0f;

        // CALCULATE ACCENT VALUES
        // glm::vec3 AccentBase = glm::vec3(0.000f, 0.729f, 0.380f);   // GREEN
        // glm::vec3 AccentBase = glm::vec3(0.608f, 0.000f, 0.620f);   // VIOLET
        glm::vec3 AccentBase = glm::vec3(0.000f, 0.621f, 0.464f);   // SEA-GREEN
        glm::vec3 AccentDark = AccentBase * 0.90f;
        glm::vec3 AccentLight = AccentBase * 1.15f;

        // TEXT
        colors[ImGuiCol_Text]                       = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled]               = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

        // TITLE (PANEL)
        colors[ImGuiCol_TitleBg]                    = ImVec4(AccentBase.r, AccentBase.g, AccentBase.b, 1.00f);
        colors[ImGuiCol_TitleBgActive]              = ImVec4(AccentBase.r, AccentBase.g, AccentBase.b, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]           = ImVec4(AccentBase.r, AccentBase.g, AccentBase.b, 1.00f);

        // WINDOW BACKGROUNDS
        colors[ImGuiCol_WindowBg]                   = ImVec4(0.06f, 0.06f, 0.06f, 0.95f);
        colors[ImGuiCol_ChildBg]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg]                    = ImVec4(0.08f, 0.08f, 0.08f, 0.95f);

        // HEADERS
        colors[ImGuiCol_Header]                     = ImVec4(AccentDark.r, AccentDark.g, AccentDark.b, 0.31f);
        colors[ImGuiCol_HeaderHovered]              = ImVec4(AccentDark.r, AccentDark.g, AccentDark.b, 0.80f);
        colors[ImGuiCol_HeaderActive]               = ImVec4(AccentDark.r, AccentDark.g, AccentDark.b, 0.65f);

        // SLIDERS / FRAMES
        colors[ImGuiCol_FrameBg]                    = ImVec4(0.16f, 0.16f, 0.16f, 0.54f);
        colors[ImGuiCol_FrameBgHovered]             = ImVec4(0.26f, 0.26f, 0.26f, 0.40f);
        colors[ImGuiCol_FrameBgActive]              = ImVec4(0.26f, 0.26f, 0.26f, 0.67f);

        colors[ImGuiCol_SliderGrab]                 = ImVec4(AccentBase.r, AccentBase.g, AccentBase.b, 1.00f);
        colors[ImGuiCol_SliderGrabActive]           = ImVec4(AccentLight.r, AccentLight.g, AccentLight.b, 1.00f);

        // BUTTONS
        colors[ImGuiCol_Button]                     = ImVec4(0.16f, 0.16f, 0.16f, 0.54f);
        colors[ImGuiCol_ButtonHovered]              = ImVec4(0.26f, 0.26f, 0.26f, 0.40f);
        colors[ImGuiCol_ButtonActive]               = ImVec4(0.26f, 0.26f, 0.26f, 0.67f);

        // CHECK MARKS
        colors[ImGuiCol_CheckMark]                  = ImVec4(AccentBase.r, AccentBase.g, AccentBase.b, 1.00f);

        // SCROLLBAR
        colors[ImGuiCol_ScrollbarBg]                = ImVec4(0.25f, 0.25f, 0.25f, 0.60f);
        colors[ImGuiCol_ScrollbarGrab]              = ImVec4(0.53f, 0.53f, 0.53f, 0.30f);
        colors[ImGuiCol_ScrollbarGrabHovered]       = ImVec4(0.53f, 0.53f, 0.53f, 0.40f);
        colors[ImGuiCol_ScrollbarGrabActive]        = ImVec4(0.53f, 0.53f, 0.53f, 0.60f);

        // RESIZE
        colors[ImGuiCol_ResizeGrip]                 = ImVec4(AccentBase.r, AccentBase.g, AccentBase.b, 0.20f);
        colors[ImGuiCol_ResizeGripHovered]          = ImVec4(AccentBase.r, AccentBase.g, AccentBase.b, 0.67f);
        colors[ImGuiCol_ResizeGripActive]           = ImVec4(AccentBase.r, AccentBase.g, AccentBase.b, 0.95f);

        // SEPERATORS (WINDOW RESIZE ALSO)
        style.Colors[ImGuiCol_Separator]            = ImVec4(AccentBase.r, AccentBase.g, AccentBase.b, 1.00f);
        style.Colors[ImGuiCol_SeparatorHovered]     = ImVec4(AccentBase.r, AccentBase.g, AccentBase.b, 0.78f);
        style.Colors[ImGuiCol_SeparatorActive]      = ImVec4(AccentBase.r, AccentBase.g, AccentBase.b, 1.00f);
    }

    void RenderMainPanel(Application::AppContext* appContext) {
        ImVec2 minSize(float(MINIMUM_WINDOW_WIDTH) / 2.0f, float(MINIMUM_WINDOW_HEIGHT) / 2.0f);
        ImVec2 maxSize(FLT_MAX, FLT_MAX);
        ImGui::SetNextWindowSize(minSize, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(minSize, maxSize);
        ImGui::Begin("##CONTROL_PANEL", nullptr, ImGuiWindowFlags_NoCollapse);

        ImGui::Button("Placeholder");

        ImGui::End();
    }

}