#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <TileRenderer.hpp>

// BOUND FOR THE MIN ZOOM ALLOWED,
// PERFORMANCE CONCERN IF LOADING TOO MANY TILES
constexpr double MAX_VISIBLE_TILES = 200.0;

class Camera {
    public:
        Camera(int windowWidth, int windowHeight, TileCenter center, WorldBounds bounds) : 
            width(windowWidth), height(windowHeight) {
            UpdateProjection();
            UpdateView();
            
            // INITIALIZE CAMERA TO THE TILE DATA
            SetBounds(bounds);

            SetPosition(glm::vec2(
                static_cast<float>(center.x), 
                static_cast<float>(center.y)
            ));
        }
        ~Camera() = default;

        void Update(float deltaTime);

        void Resize(int windowWidth, int windowHeight);

        ViewBounds GetVisibleBounds() const;

        void ProcessMouseMotion(float xrel, float yrel);

        void ProcessMouseWheel(float amount);

        void SetPosition(const glm::vec2& position);

        void ClampToBounds();

        void SetBounds(const WorldBounds& bounds);

        float GetZoom() const { return zoom; }

        glm::mat4 GetViewProjection() const { return projection * view; }

    private:
        void UpdateView();

        void UpdateProjection();

    private:
        int width = 1280;
        int height = 720;

        float zoom = 0.5f;
        float minZoom = 0.1f;
        float maxZoom = 1.0f;

        bool hasBounds = false;

        WorldBounds worldBounds{};

        glm::mat4 view{1.0f};
        glm::mat4 projection{1.0f};

        glm::vec2 position{0.0f, 0.0f};
};