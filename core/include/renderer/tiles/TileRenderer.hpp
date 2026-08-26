#pragma once

#include <string>
#include <unordered_map>
#include <cstddef>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <TileReader.hpp>

constexpr double ORIGIN_SHIFT = 20037508.342789244;
constexpr double WORLD_SIZE   = ORIGIN_SHIFT * 2.0;

struct TileKey {
    int x, y, z;
    
    bool operator==(const TileKey& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct TileKeyHash {
    size_t operator()(const TileKey& key) const {
        return
            (static_cast<size_t>(key.x) << 40) ^
            (static_cast<size_t>(key.y) << 20) ^ 
            (static_cast<size_t>(key.z));
    }
};

struct CachedTexture {
    GLuint texture = 0;
    uint64_t lastUsed = 0;
};

struct TileCenter {
    double x;
    double y;
};

struct WorldBounds {
    double minX;
    double minY;
    double maxX;
    double maxY;
};

struct ViewBounds {
    double minX;
    double minY;
    double maxX;
    double maxY;
};

class TileRenderer {
    public:
        TileRenderer() = default;
        ~TileRenderer() { Shutdown(); };

        bool Init();

        bool InitDebug();

        bool LoadTiles(const std::string& filename);

        void Render(const glm::mat4& viewProjection, const ViewBounds& cameraBounds, float cameraZoom);

        void PurgeUnusedTextures(uint64_t maxAgeTicks);

        int CalculateTileZoomLevel(float cameraZoom) const;

        int GetTileZoomLevel() const { return tileZoomLevel; };

        TileCenter GetCenter();

        WorldBounds GetWorldBounds();

        MapBounds GetMapBounds() const { return reader.GetBounds(); };

        int GetTileCount() const { return tileCount; };

        bool& GetTileBorderEnabled() { return tileBorderEnabled; };

    private:
        GLuint LoadTexture(int z, int x, int y);

        void DrawTile(int zoomLevel, int x, int y, GLuint texture, const glm::mat4& viewProjection);

        void DrawTileBorder(int zoomLevel, int x, int y, const glm::mat4& viewProjection);

        void Shutdown();

    private:
        TileReader reader;

        int tileCount = 0;
        int tileZoomLevel = 0;

        GLuint tileProgram = 0;
        GLuint tileVAO = 0;
        GLuint tileVBO = 0;

        bool tileBorderEnabled = false;
        GLuint debugProgram = 0;
        GLuint debugVAO = 0;
        GLuint debugVBO = 0;

        std::unordered_map<TileKey, CachedTexture, TileKeyHash> textureCache;
};