#include <memory>
#include <vector>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <App.hpp>
#include <AppContext.hpp>
#include <Camera.hpp>
#include <TextRenderer.hpp>
#include <MapRenderer.hpp>
#include <UserInterface.hpp>

namespace Application {

    bool App::CreateSDLWindow(const char* title) {
        // SET SDL GL ATTRIBUTES (OpenGL 4.5 CORE)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        
        SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);

        window = SDL_CreateWindow(title, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FLAGS); 
        return window != nullptr;
    }

    bool App::CreateGLContext(bool enableVsync) {
        glContext = SDL_GL_CreateContext(window);
        SDL_GL_SetSwapInterval(enableVsync ? 1 : 0);
        return gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);
    }

    SDL_AppResult App::Init(int argc, char** argv) {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) return SDL_APP_FAILURE;
        if (!CreateSDLWindow("Map Viewer")) return SDL_APP_FAILURE;
        if (!CreateGLContext(false)) return SDL_APP_FAILURE;

        TTF_Init();
        TTF_Font* textFont = TTF_OpenFont("../assets/fonts/Roboto-Regular.ttf", 18.0f);
        appContext.textRenderer = std::make_unique<TextRenderer>();
        appContext.textRenderer->Init(textFont);

        appContext.camera = std::make_unique<Camera>(WINDOW_WIDTH, WINDOW_HEIGHT);

        appContext.geoJsonReader = std::make_unique<GeoJsonReader>();
        appContext.geoJsonReader->open("../data/raster_test.geojson");

        appContext.mapRenderer = std::make_unique<MapRenderer>();
        appContext.mapRenderer->Init();
        if (!appContext.mapRenderer->LoadGeoJson(*appContext.geoJsonReader)) {
            return SDL_APP_FAILURE;
        }
        GeoBounds bounds = appContext.geoJsonReader->GetBounds();
        appContext.camera->FitBounds(
            bounds.minX,
            bounds.minY,
            bounds.maxX,
            bounds.maxY
        );
        SDL_Log("Bounds: %lf, %lf -> %lf, %lf",
            bounds.minX,
            bounds.minY,
            bounds.maxX,
            bounds.maxY
        );
        
        // INITIALIZE IMGUI
        IMGUI_CHECKVERSION();
        ImGui::CreateContext(); 
        ImGuiIO& io = ImGui::GetIO();
        appContext.fontBold = io.Fonts->AddFontFromFileTTF("../assets/fonts/Roboto-Bold.ttf", 16.0f);
        appContext.fontRegular = io.Fonts->AddFontFromFileTTF("../assets/fonts/Roboto-Regular.ttf", 16.0f);
        io.FontDefault = appContext.fontRegular;
        UserInterface::SetCustomTheme();

        ImGui_ImplSDL3_InitForOpenGL(window, glContext);
        ImGui_ImplOpenGL3_Init(GLSL_VERSION);

        lastTime = SDL_GetPerformanceCounter();
        return SDL_APP_CONTINUE;
    }

    SDL_AppResult App::ProcessEvent(SDL_Event* event) {
        switch (event->type) {
            case SDL_EVENT_QUIT:
                return SDL_APP_SUCCESS;
            case SDL_EVENT_MOUSE_WHEEL:
                appContext.camera->ProcessMouseWheel(event->wheel.y);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event->button.button == SDL_BUTTON_LEFT)
                    appContext.dragging = true;
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event->button.button == SDL_BUTTON_LEFT)
                    appContext.dragging = false;
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (appContext.dragging)
                    appContext.camera->ProcessMouseMotion(
                        event->motion.xrel,
                        event->motion.yrel
                    );
                break;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                width = event->window.data1;
                height = event->window.data2;
                glViewport(0, 0, width, height);
                break;
        }

        ImGui_ImplSDL3_ProcessEvent(event);
        return SDL_APP_CONTINUE;
    }

    void App::RenderScene(float deltaTime) {
        // DRAW BACKGROUND/CLEAR FRAMEBUFFER
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        appContext.textRenderer->UpdateFPS();
        appContext.textRenderer->Render(width, height);

        appContext.mapRenderer->Render(appContext.camera->GetViewProjection());
    }

    SDL_AppResult App::Frame() {
        // UPDATE DELTA TIME
        uint64_t currentTime = SDL_GetPerformanceCounter();
        float deltaTime = float(currentTime - lastTime) / SDL_GetPerformanceFrequency();
        lastTime = currentTime;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // RENDER GUI
        UserInterface::RenderMainPanel(&appContext);

        // RENDER SCENE (CAMERA, CUBE, TEXT, ...)
        RenderScene(deltaTime);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);

        return SDL_APP_CONTINUE;
    }

    void App::Shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

}
