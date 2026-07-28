#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
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

void Camera::UpdateProjectionFromBounds() {
    if (!hasBounds)
        return;

    float mapWidth = static_cast<float>(boundsMaxX - boundsMinX);
    float mapHeight = static_cast<float>(boundsMaxY - boundsMinY);

    constexpr float FIT_ZOOM = 0.65f;
    mapWidth *= FIT_ZOOM;
    mapHeight *= FIT_ZOOM;

    float viewAspect = static_cast<float>(width) / static_cast<float>(height);
    float mapAspect = mapWidth / mapHeight;

    if (mapAspect > viewAspect) {
        mapHeight = mapWidth / viewAspect;
    } else {
        mapWidth = mapHeight * viewAspect;
    }

    projection = glm::ortho(
        -mapWidth * 0.5f, mapWidth * 0.5f,
        -mapHeight * 0.5f, mapHeight * 0.5f,
        -1.0f, 1.0f
    );
}

void Camera::FitBounds(double minX, double minY, double maxX, double maxY) {
    boundsMinX = minX;
    boundsMinY = minY;
    boundsMaxX = maxX;
    boundsMaxY = maxY;

    hasBounds = true;

    position.x = static_cast<float>((minX + maxX) * 0.5);
    position.y = static_cast<float>((minY + maxY) * 0.5);

    UpdateProjectionFromBounds();
    UpdateView();
}

void Camera::Resize(int windowWidth, int windowHeight) {
    width = windowWidth;
    height = windowHeight;

    if (hasBounds) {
        UpdateProjectionFromBounds();
    } else {
        UpdateProjection();
    }
    UpdateView();
}

// void Camera::ProcessMouseMotion(float xrel, float yrel) {
//     float worldWidth = 2.0f / projection[0][0];
//     float worldHeight = 2.0f / projection[1][1];

//     position.x -= xrel * worldWidth / width;
//     position.y += yrel * worldHeight / height;

//     UpdateView();
// }

void Camera::SetPosition(const glm::vec2& newPosition) {
    position = newPosition;
    UpdateView();
}

void Camera::UpdateProjection() {
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;

    projection = glm::ortho(
        -halfWidth, halfWidth,
        -halfHeight, halfHeight,
        -1.0f, 1.0f
    );
}