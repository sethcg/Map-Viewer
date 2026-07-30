#include <algorithm>

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
        -halfWidth,
         halfWidth,
        -halfHeight,
         halfHeight,
        -1.0f,
         1.0f
    );
}

void Camera::Resize(int w, int h) {
    width = w;
    height = h;

    UpdateProjection();
}

void Camera::ProcessMouseMotion(float xrel, float yrel) {
    float worldWidth = width / zoom;
    float worldHeight = height / zoom;

    position.x -= xrel * worldWidth / width;
    position.y += yrel * worldHeight / height;

    UpdateView();
}

void Camera::ProcessMouseWheel(float wheel) {
    if (wheel > 0)
        zoom *= 1.15f;
    else if (wheel < 0)
        zoom /= 1.15f;

    zoom = glm::clamp(zoom, 0.1f, 100.0f);
    UpdateProjection();
}

void Camera::SetPosition(const glm::vec2& pos) {
    position = pos;
    UpdateView();
}

void Camera::SetZoom(float value) {
    zoom = glm::clamp(value, 0.1f, 100.0f);
    UpdateProjection();
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