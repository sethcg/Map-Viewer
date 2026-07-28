#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
    public:
        Camera(int windowWidth, int windowHeight){
            Resize(windowWidth, windowHeight);
        }
        virtual ~Camera() = default;

        void Update(float deltaTime);

        void FitBounds(double minX, double minY, double maxX, double maxY);

        void Resize(int windowWidth, int windowHeight);

        // void ProcessMouseMotion(float xrel, float yrel);
        // void ProcessMouseWheel(float amount);

        void SetPosition(const glm::vec2& position);

        glm::vec2 GetPosition() const {
            return position;
        }

        glm::mat4 GetViewProjection() const {
            return projection * view;
        }


    private:
        void UpdateView();

        void UpdateProjection();

        void UpdateProjectionFromBounds();

        int width = 1280;
        int height = 720;

        bool hasBounds = false;

        double boundsMinX = 0;
        double boundsMinY = 0;
        double boundsMaxX = 0;
        double boundsMaxY = 0;

        glm::mat4 view{1.0f};
        glm::mat4 projection{1.0f};
        glm::vec2 position{ 0.0f, 0.0f };
};