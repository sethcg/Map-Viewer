#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <TextRenderer.hpp>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_FLAGS SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE

#define MINIMUM_WINDOW_WIDTH 640
#define MINIMUM_WINDOW_HEIGHT 360

#define GLSL_VERSION "#version 330"

namespace Application {

    struct AppContext {
        std::string filepath;
        
        ImFont* fontBold;
        ImFont* fontRegular;
        
        std::unique_ptr<TextRenderer> textRenderer;

        AppContext() {
            filepath = "";
        }

    };

}
