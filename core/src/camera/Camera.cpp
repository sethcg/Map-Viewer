#include <algorithm>

#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Camera.hpp>

void Camera::Update(float deltaTime) {
    UpdateView();
}

void Camera::UpdateView() {
    view = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(-position.x, -position.y, 0.0f)
    );
}

void Camera::UpdateProjection() {
    float halfWidth = (width * 0.5f) / zoom;
    float halfHeight = (height * 0.5f) / zoom;

    projection = glm::ortho(
        -halfWidth, halfWidth,
        -halfHeight, halfHeight,
        -1.0f, 1.0f
    );
}

void Camera::Resize(int w, int h) {
    width = w;
    height = h;

    UpdateProjection();
    ClampToBounds();
}

void Camera::ProcessMouseMotion(float xrel, float yrel) {
    float worldWidth = width / zoom;
    float worldHeight = height / zoom;

    position.x -= xrel * worldWidth / width;
    position.y += yrel * worldHeight / height;

    UpdateView();
    ClampToBounds();
}

void Camera::ProcessMouseWheel(float wheel) {
    constexpr float zoomStep = 0.05f;

    if (wheel > 0.0f) {
        zoom += zoomStep;
    } else if (wheel < 0.0f) {
        zoom -= zoomStep;
    }

    zoom = glm::clamp(zoom, minZoom, maxZoom);

    UpdateProjection();
    ClampToBounds();
}

void Camera::SetPosition(const glm::vec2& pos) {
    position = pos;
    UpdateView();
}

void Camera::ClampToBounds() {
    if (!hasBounds) return;

    float halfWidth  = width * 0.5f / zoom;
    float halfHeight = height * 0.5f / zoom;

    float minX = worldBounds.minX + halfWidth;
    float maxX = worldBounds.maxX - halfWidth;

    float minY = worldBounds.minY + halfHeight;
    float maxY = worldBounds.maxY - halfHeight;

    if (minX <= maxX)
        position.x = std::clamp(position.x, minX, maxX);
    else
        position.x = (worldBounds.minX + worldBounds.maxX) * 0.5f;

    if (minY <= maxY)
        position.y = std::clamp(position.y, minY, maxY);
    else
        position.y = (worldBounds.minY + worldBounds.maxY) * 0.5f;

    UpdateView();
}

void Camera::SetBounds(const WorldBounds& bounds) {
    hasBounds = true;
    worldBounds = bounds;

    ClampToBounds();
}

ViewBounds Camera::GetVisibleBounds() const {
    float viewWidth = width / zoom;
    float viewHeight = height / zoom;

    return {
        position.x - viewWidth * 0.5f,
        position.y - viewHeight * 0.5f,
        position.x + viewWidth * 0.5f,
        position.y + viewHeight * 0.5f
    };
}