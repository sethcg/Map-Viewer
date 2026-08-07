#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Camera.hpp>
#include <GeoJsonReader.hpp>
#include <TextRenderer.hpp>
#include <TileRenderer.hpp>
#include <PolygonRenderer.hpp>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_FLAGS SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE

#define MINIMUM_WINDOW_WIDTH 640
#define MINIMUM_WINDOW_HEIGHT 360

#define GLSL_VERSION "#version 450"

namespace Application {

    struct AppContext {
        std::string filepath;
        
        ImFont* fontBold;
        ImFont* fontRegular;
        
        bool dragging;

        std::unique_ptr<Camera> camera;
        std::unique_ptr<GeoJsonReader> geoJsonReader;
        std::unique_ptr<TileRenderer> tileRenderer;
        std::unique_ptr<PolygonRenderer> polygonRenderer;
        // std::unique_ptr<TextRenderer> textRenderer;

        AppContext() {
            filepath = "";
            dragging = false;
        }

    };

}
