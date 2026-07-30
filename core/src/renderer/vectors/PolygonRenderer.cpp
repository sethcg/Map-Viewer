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
#include <PolygonRenderer.hpp>
#include <RendererHelper.hpp>

bool PolygonRenderer::Init() {
    mapShader = Renderer::CreateShaderProgramFromFiles(
        "../assets/shaders/map/map.vert",
        "../assets/shaders/map/map.frag"
    );

    glUseProgram(mapShader);
    glCreateVertexArrays(1, &vao);
    glCreateBuffers(1, &vbo);
    glCreateBuffers(1, &ebo);
    glUseProgram(0);

    return true;
}

bool PolygonRenderer::LoadGeoJson(GeoJsonReader& reader) {
    auto features = reader.readAll();

    if (features.empty()) {
        SDL_Log("GEOJSON CONTAINS NO FEATURES\n");
        return false;
    }

    BuildMesh(features);
    UploadMesh();

    return true;
}

void PolygonRenderer::BuildMesh(const std::vector<GeoFeature>& features) {
    vertices.clear();
    indices.clear();

    for (const auto& feature : features) {
        AddFeature(feature);
    }
}

void PolygonRenderer::AddFeature(const GeoFeature& feature) {
    switch (feature.type) {

    case GeometryType::Point:
    {
        uint32_t start = static_cast<uint32_t>(vertices.size());
        for (const auto& point : feature.vertices) {
            vertices.push_back({
                glm::vec2(point.x, point.y),
                static_cast<float>(feature.id)
            });
            indices.push_back(start++);
        }
        break;
    }

    case GeometryType::LineString:
    {
        uint32_t start = static_cast<uint32_t>(vertices.size());
        for (const auto& point : feature.vertices) {
            vertices.push_back({
                glm::vec2(point.x, point.y),
                static_cast<float>(feature.id)
            });
        }
        for (uint32_t i = 0; i + 1 < feature.vertices.size(); i++) {
            indices.push_back(start + i);
            indices.push_back(start + i + 1);
        }
        break;
    }

    case GeometryType::Polygon:
    {
        bool hasHoles = feature.ringStarts.size() > 1;
        if (!hasHoles) {
            // FAST PATH: SIMPLE POLYGON
            uint32_t start = static_cast<uint32_t>(vertices.size());
            for (const auto& point : feature.vertices) {
                vertices.push_back({
                    glm::vec2(point.x, point.y),
                    static_cast<float>(feature.id)
                });
            }
            for (uint32_t i = 1; i + 1 < feature.vertices.size(); i++) {
                indices.push_back(start);
                indices.push_back(start + i);
                indices.push_back(start + i + 1);
            }
        } else {
            // SLOW PATH: POLYGON WITH HOLES
            TriangulateWithEarcut(feature);
        }
        break;
    }

    }
}

void PolygonRenderer::TriangulateWithEarcut(const GeoFeature& feature) {
    std::vector<std::vector<std::array<double, 2>>> polygon;
    polygon.reserve(feature.ringStarts.size());

    size_t totalVertices = 0;

    for (size_t r = 0; r < feature.ringStarts.size(); ++r) {
        size_t start = feature.ringStarts[r];
        size_t end = (r + 1 < feature.ringStarts.size())
            ? feature.ringStarts[r + 1]
            : feature.vertices.size();

        std::vector<std::array<double, 2>> ring;
        ring.reserve(end - start);

        for (size_t i = start; i < end; ++i) {
            ring.push_back({
                feature.vertices[i].x,
                feature.vertices[i].y
            });
        }

        if (ring.size() >= 3) {
            totalVertices += ring.size();
            polygon.push_back(std::move(ring));
        }
    }

    if (polygon.empty())
        return;

    SDL_Log("POLYGON: %zu RINGS, %zu VERTICES", polygon.size(), totalVertices);

    constexpr size_t MAX_RINGS = 200;
    constexpr size_t MAX_VERTICES = 50000;

    if (polygon.size() > MAX_RINGS || totalVertices > MAX_VERTICES) {
        SDL_Log("SKIPPING EARCUT (%zu RINGS, %zu VERTICES)", polygon.size(), totalVertices);
        return;
    }
    uint32_t vertexStart = static_cast<uint32_t>(vertices.size());

    // Copy vertices directly from the feature (avoids iterating polygon again)
    vertices.reserve(vertices.size() + feature.vertices.size());
    for (const auto& p : feature.vertices) {
        vertices.push_back({
            glm::vec2(static_cast<float>(p.x), static_cast<float>(p.y)),
            static_cast<float>(feature.id)
        });
    }

    auto t0 = SDL_GetPerformanceCounter();
    auto triangles = mapbox::earcut<uint32_t>(polygon);
    auto t1 = SDL_GetPerformanceCounter();

    double ms = (t1 - t0) * 1000.0 / static_cast<double>(SDL_GetPerformanceFrequency());
    SDL_Log("EARCUT: %.2f ms (%zu TRIANGLES)", ms, triangles.size() / 3);

    indices.reserve(indices.size() + triangles.size());
    for (uint32_t index : triangles) {
        indices.push_back(vertexStart + index);
    }
}

void PolygonRenderer::UploadMesh() {
    if (vertices.empty() || indices.empty()) {
        return;
    }
    SDL_Log(
        "VERTICES: %zu INDICES: %zu TRIANGLES: %zu",
        vertices.size(),
        indices.size(),
        indices.size() / 3
    );

    // VERTEX BUFFER
    glNamedBufferData(
        vbo,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
        vertices.data(),
        GL_STATIC_DRAW
    );

    // INDEX BUFFER
    glNamedBufferData(
        ebo,
        static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)),
        indices.data(),
        GL_STATIC_DRAW
    );
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));
    glVertexArrayElementBuffer(vao, ebo);

    // position
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(
        vao,
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        offsetof(Vertex, position)
    );
    glVertexArrayAttribBinding(vao, 0, 0);


    // polygon ID
    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribFormat(
        vao,
        1,
        1,
        GL_FLOAT,
        GL_FALSE,
        offsetof(Vertex, polygonID)
    );
    glVertexArrayAttribBinding(vao, 1, 0);

    glVertexArrayAttribBinding(vao, 0, 0);
}

void PolygonRenderer::Render(const glm::mat4& projection) {
    if (!mapShader || indices.empty()) {
        return;
    }

    glUseProgram(mapShader);

    GLint projectionLocation = glGetUniformLocation(mapShader, "projection");
    if (projectionLocation >= 0) {
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
    }

    GLint colorLocation = glGetUniformLocation(mapShader, "color");
    if (colorLocation >= 0) {
        glm::vec3 color(0.2f, 0.7f, 0.3f);
        glUniform3fv(colorLocation, 1, glm::value_ptr(color));
    }

    glBindVertexArray(vao);
    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(indices.size()),
        GL_UNSIGNED_INT,
        nullptr
    );
    glBindVertexArray(0);
}

void PolygonRenderer::Shutdown() {
    vertices.clear();
    indices.clear();

    if (mapShader) glDeleteProgram(mapShader);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ebo) glDeleteBuffers(1, &ebo);
    mapShader = vao = vbo = ebo = 0;
}