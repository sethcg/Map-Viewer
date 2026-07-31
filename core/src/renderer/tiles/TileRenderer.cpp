#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL.h>
#include <stb_image.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Camera.hpp>
#include <TileReader.hpp>
#include <RendererHelper.hpp>
#include <TileRenderer.hpp>

int currentTileCount = 0;

namespace {
    int WorldToTileX(double x, int z) {
        double u = (x + ORIGIN_SHIFT) / WORLD_SIZE;
        return (int)std::floor(u * (1 << z));
    }

    int WorldToTileY(double y, int z) {
        double v = (ORIGIN_SHIFT - y) / WORLD_SIZE;
        return (int)std::floor(v * (1 << z));
    }

    double LonToWorldX(double lon) {
        return lon * ORIGIN_SHIFT / 180.0;
    }

    double LatToWorldY(double lat) {
        double latRad = lat * M_PI / 180.0;
        return ORIGIN_SHIFT * std::log(std::tan(M_PI / 4.0 + latRad / 2.0)) / M_PI;
    }
}

bool TileRenderer::Init() {
    tileProgram = Renderer::CreateShaderProgramFromFiles(
        "../assets/shaders/tile/tile.vert",
        "../assets/shaders/tile/tile.frag"
    );

    if (!tileProgram) {
        SDL_Log("FAILED TO CREATE TILE SHADER");
        return false;
    }

    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 1.0f,
        1.0f,  1.0f, 1.0f, 0.0f,

        -1.0f, -1.0f, 0.0f, 1.0f,
        1.0f,  1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        nullptr
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        (void*)(2 * sizeof(float))
    );

    glBindVertexArray(0);
    return true;
}

bool TileRenderer::LoadTiles(const std::string& filename) {
    return reader.Open(filename);
}

GLuint TileRenderer::LoadTexture(int z, int x, int y) {
    TileKey key{x, y, z};

    auto found = textureCache.find(key);
    if (found != textureCache.end()) {
        found->second.lastUsed = SDL_GetPerformanceCounter();
        return found->second.texture;
    }

    std::vector<uint8_t> data;
    if (!reader.GetTile(z, x, y, data)) {
        // EXPECTED WHEN THE CAMERA IS OUTSIDE THE MAP BOUNDS
        // TODO: FIX BY CLAMPING THE CAMERA TO THE TILEMAP BOUNDS SET IN THE METADATA
        SDL_Log("MISSING TILE: %d/%d/%d", z, x, y);
        return 0;
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* pixels =
        stbi_load_from_memory(
            data.data(),
            static_cast<int>(data.size()),
            &width,
            &height,
            &channels,
            STBI_rgb_alpha
        );

        SDL_Log("TILES LOADED: %d", currentTileCount++);
        // SDL_Log("LOADED TILE %dx%d CHANNELS=%d FIRST PIXEL RGBA: %d %d %d %d",
        //     width, height, channels,
        //     pixels[0],
        //     pixels[1],
        //     pixels[2],
        //     pixels[3]
        // );

    if (!pixels) {
        SDL_Log("STB_IMAGE FAILED: %s", stbi_failure_reason());
        return 0;
    }

    GLuint texture = 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE
    );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels
    );

    stbi_image_free(pixels);

    if (glGetError() != GL_NO_ERROR) {
        SDL_Log("OPENGL TEXTURE UPLOAD FAILED");
        glDeleteTextures(1, &texture);
        return 0;
    }
    textureCache.emplace(
        key, CachedTexture{ 
            texture,
            SDL_GetPerformanceCounter()
        }
    );

    return texture;
}

void TileRenderer::Render(const glm::mat4& viewProjection, const ViewBounds& cameraBounds) {
    int minX = WorldToTileX(cameraBounds.minX, zoomLevel);
    int maxX = WorldToTileX(cameraBounds.maxX, zoomLevel);
    int minY = WorldToTileY(cameraBounds.maxY, zoomLevel);
    int maxY = WorldToTileY(cameraBounds.minY, zoomLevel);

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            GLuint texture = LoadTexture(zoomLevel, x, y);

            if (!texture) continue;

            DrawTile(x, y, texture, viewProjection);
        }
    }
}

TileCenter TileRenderer::GetCenter() {
    TileCenter center{};

    MapCenter mapCenter = reader.GetCenter();

    center.x = LonToWorldX(mapCenter.lon);
    center.y = LatToWorldY(mapCenter.lat);

    return center;
}

WorldBounds TileRenderer::GetWorldBounds() {
    MapBounds map = reader.GetBounds();
    
    return {
        LonToWorldX(map.west),
        LatToWorldY(map.south),
        LonToWorldX(map.east),
        LatToWorldY(map.north)
    };
}

void TileRenderer::DrawTile(int x, int y, GLuint texture, const glm::mat4& vp) {
    const double tiles = static_cast<double>(1 << zoomLevel);
    const double tileSize = WORLD_SIZE / tiles;

    const double minX = -ORIGIN_SHIFT + x * tileSize;
    const double maxY = ORIGIN_SHIFT - y * tileSize;

    glm::mat4 model(1.0f);

    // Move tile center into world coordinates
    model = glm::translate(model, glm::vec3(minX + tileSize * 0.5, maxY - tileSize * 0.5, 0.0f));

    // Quad is -1..1, so scale by half tile size
    constexpr double overlap = 1.001;
    model = glm::scale(model, glm::vec3(
        tileSize * 0.5 * overlap, 
        tileSize * 0.5 * overlap, 
        1.0f
    ));

    glUseProgram(tileProgram);

    glUniformMatrix4fv(
        glGetUniformLocation(tileProgram, "uVP"),
        1,
        GL_FALSE,
        glm::value_ptr(vp)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(tileProgram, "uModel"),
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glUniform1i(glGetUniformLocation(tileProgram, "tileTexture"), 0);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void TileRenderer::Shutdown() {
    for (auto& [key, texture] : textureCache) {
        if (texture.texture) glDeleteTextures(1, &texture.texture);
    }

textureCache.clear();
    textureCache.clear();

    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (tileProgram) glDeleteProgram(tileProgram);
}