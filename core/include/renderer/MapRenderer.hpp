#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include <string>
#include <iostream>

#include <mapbox/earcut.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <GeoJsonReader.hpp>
#include <RendererHelper.hpp>

class MapRenderer {
    public:
        MapRenderer() = default;
        ~MapRenderer() { Shutdown(); }

        bool Init();

        void Shutdown();

        bool LoadGeoJson(GeoJsonReader& reader);

        void Render(const glm::mat4& projection);

    private:
        void BuildMesh(const std::vector<GeoFeature>& features);
        
        void AddFeature(const GeoFeature& feature);

        void UploadMesh();

        void TriangulateWithEarcut(const GeoFeature& feature);

    private:
        struct Vertex { glm::vec2 position; };

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;
        GLuint mapShader = 0;

    private:
        MapRenderer(const MapRenderer&) = delete;
        MapRenderer& operator=(const MapRenderer&) = delete;

};