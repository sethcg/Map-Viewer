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

    static bool showSidebar = true;

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

    void RenderDebug(Application::AppContext* appContext) {
        static const MapBounds mapBounds = appContext->tileRenderer->GetMapBounds();
        static float fps = 0.0f;
        static float timer = 0.0f;
        static int tile_count = 0;

        timer += ImGui::GetIO().DeltaTime;
        if (timer >= 0.5f) {
            tile_count = appContext->tileRenderer->GetTileCount();
            fps = ImGui::GetIO().Framerate;
            timer = 0.0f;
        }

        ImGui::Indent();

        ImGui::Text("FPS: %.1f", fps);
        ImGui::Text("TILES LOADED: %d", tile_count);

        ImGui::Text("MAP BOUNDS");
        ImGui::Indent();
        if (ImGui::BeginTable("MAP_BOUNDS", 2, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("NORTH:");
            ImGui::TableNextColumn(); ImGui::Text("%.8f", mapBounds.north);

            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("SOUTH:");
            ImGui::TableNextColumn(); ImGui::Text("%.8f", mapBounds.south);

            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("EAST:");
            ImGui::TableNextColumn(); ImGui::Text("%.8f", mapBounds.east);

            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Text("WEST:");
            ImGui::TableNextColumn(); ImGui::Text("%.8f", mapBounds.west);

            ImGui::EndTable();
        }
        ImGui::Unindent();

        ImGui::Unindent();
    }

    void RenderSidebar(Application::AppContext* appContext) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->WorkPos,ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(SIDEBAR_WIDTH, viewport->WorkSize.y));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin(
            "CONTROL_PANEL", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBackground
        );
        ImGui::PopStyleVar();

        const char* header = "CONTROLS";
        ImGuiID headerId = ImGui::GetID(header);
        bool isOpen = ImGui::GetStateStorage()->GetBool(headerId, true);

        // ADD THE SEMI-TRANSPARENT BACKGROUND
        if(isOpen) {
            ImVec2 min_point = ImGui::GetCursorScreenPos();
            ImVec2 max_point = ImVec2(min_point.x + SIDEBAR_WIDTH, min_point.y + viewport->WorkSize.y);
            ImGui::GetWindowDrawList()->AddRectFilled(
                min_point, max_point,
                ImGui::GetColorU32(ImVec4(0.06f, 0.06f, 0.06f, 0.85f)),
                ImGui::GetStyle().ChildRounding
            );
        }

        ImGui::PushFont(appContext->fontBold);
        ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.00f, 0.62f, 0.46f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.00f, 0.70f, 0.52f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0.00f, 0.55f, 0.40f, 1.00f));
        if (ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_DefaultOpen)) { 
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            float contentHeight = viewport->WorkSize.y - ImGui::GetCursorPosY();
            ImGui::BeginChild("CONTENT", ImVec2(0, contentHeight), false);

            // if (ImGui::CollapsingHeader("TEST", ImGuiTreeNodeFlags_DefaultOpen)) {

            // }

            RenderDebug(appContext);
            
            ImGui::EndChild();
        } else {
            ImGui::PopFont();
            ImGui::PopStyleColor(3);
        }
        
        ImGui::End();
    }

    void RenderMainPanel(Application::AppContext* appContext) {
        RenderSidebar(appContext);
    }

}