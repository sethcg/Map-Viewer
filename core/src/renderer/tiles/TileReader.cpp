#include <cstdint>
#include <string>
#include <vector>

#include <sqlite3.h>
#include <SDL3/SDL.h>

#include <TileReader.hpp>

bool TileReader::Open(const std::string& filename) {
    if (sqlite3_open_v2(filename.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        SDL_Log("FAILED TO OPEN DATABASE: %s", sqlite3_errmsg(db));
        return false;
    }

    const char* sql = "SELECT data FROM tiles WHERE z=? AND x=? AND y=?;";
    if (sqlite3_prepare_v2(db, sql, -1, &tileStmt, nullptr) != SQLITE_OK)
        return false;

    if (!LoadMetadata())
        return false;

    SDL_Log("OPENED TILE DATABASE");
    return true;
}

bool TileReader::LoadMetadata() {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT name, value FROM metadata;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* rawName = sqlite3_column_text(stmt, 0);
        const unsigned char* rawValue = sqlite3_column_text(stmt, 1);

        if (!rawName || !rawValue)
            continue;

        std::string name(reinterpret_cast<const char*>(rawName));
        std::string value(reinterpret_cast<const char*>(rawValue));

        // CENTER LONGITUDE, LATITUDE
        if (name == "center_lon")
            center.lon = std::atof(value.c_str());
        else if (name == "center_lat")
            center.lat = std::atof(value.c_str());

        // BOUNDS (LONGITUDE, LATITUDE)
        else if (name == "west")
            bounds.west = std::atof(value.c_str());
        else if (name == "south")
            bounds.south = std::atof(value.c_str());
        else if (name == "east")
            bounds.east = std::atof(value.c_str());
        else if (name == "north")
            bounds.north = std::atof(value.c_str());

        // ZOOM LEVELS
        else if (name == "min_zoom")
            zoomInfo.minZoom = std::atoi(value.c_str());
        else if (name == "max_zoom")
            zoomInfo.maxZoom = std::atoi(value.c_str());
            
        zoomInfo.centerZoom = (zoomInfo.minZoom + zoomInfo.maxZoom) / 2;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool TileReader::GetTile(int z, int x, int y, std::vector<uint8_t>& data) {
    sqlite3_reset(tileStmt);
    sqlite3_clear_bindings(tileStmt);

    sqlite3_bind_int(tileStmt, 1, z);
    sqlite3_bind_int(tileStmt, 2, x);
    sqlite3_bind_int(tileStmt, 3, y);

    if (sqlite3_step(tileStmt) != SQLITE_ROW) {
        SDL_Log("MISSING TILE (Z/X/Y): %d/%d/%d", z, x, y);
        return false;
    }

    const auto* blob = static_cast<const uint8_t*>(sqlite3_column_blob(tileStmt, 0));
    int size = sqlite3_column_bytes(tileStmt, 0);

    data.assign(blob, blob + size);

    return true;
}

void TileReader::Shutdown() {
    if (tileStmt) {
        sqlite3_finalize(tileStmt);
        tileStmt = nullptr;
    }

    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}