#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct ViewBounds {
    double minX;
    double minY;
    double maxX;
    double maxY;
};


class Camera {
    public:
        Camera(int windowWidth, int windowHeight) : width(windowWidth), height(windowHeight) {
            UpdateProjection();
            UpdateView();
        }
        ~Camera() = default;

        void Update(float deltaTime);

        void Resize(int windowWidth, int windowHeight);

        ViewBounds GetVisibleBounds() const;

        void ProcessMouseMotion(float xrel, float yrel);

        void ProcessMouseWheel(float amount);

        void SetPosition(const glm::vec2& position);

        void SetZoom(float value);

        float Camera::GetZoom() const { return zoom; }

        glm::vec2 Camera::GetPosition() const { return position; }

        glm::mat4 Camera::GetViewProjection() const { return projection * view; }

    private:
        void UpdateView();

        void UpdateProjection();

    private:
        int width = 1280;
        int height = 720;

        float zoom = 1.0f;

        float minZoom = 0.2f;
        float maxZoom = 20.0f;

        glm::mat4 view{1.0f};
        glm::mat4 projection{1.0f};

        glm::vec2 position{0.0f, 0.0f};
};