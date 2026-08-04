#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <sqlite3.h>

struct MapCenter {
    double lon = 0.0;
    double lat = 0.0;
};

struct MapBounds {
    double west;
    double south;
    double east;
    double north;
};

struct ZoomInfo {
    int minZoom = 0;
    int maxZoom = 0;
    int centerZoom = 0;
};

class TileReader {
    public:
        TileReader() = default;
        ~TileReader() { Shutdown(); };

        bool Open(const std::string& filename);

        bool LoadMetadata();

        bool GetTile(int z, int x, int y, std::vector<uint8_t>& data);

        MapCenter GetCenter() const { return center; }

        MapBounds GetBounds() const { return bounds; }

        ZoomInfo GetZoomInfo() const { return zoomInfo; }

    private:
        void Shutdown();

    private:
        sqlite3* db = nullptr;

        sqlite3_stmt* tileStmt = nullptr;

        // METADATA
        MapCenter center{};
        MapBounds bounds{};
        ZoomInfo zoomInfo{};
};