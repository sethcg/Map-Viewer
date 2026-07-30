#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <sqlite3.h>

struct MapCenter {
    double lon = 0.0;
    double lat = 0.0;
};

class TileReader {
    public:
        TileReader() = default;
        ~TileReader() { Shutdown(); };

        bool Open(const std::string& filename);

        bool GetTile(int z, int x, int y, std::vector<uint8_t>& data);

        MapCenter GetCenter() const;

    private:
        void Shutdown();

    private:
        sqlite3* db = nullptr;
};