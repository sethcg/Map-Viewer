#include <cstdint>
#include <string>
#include <vector>

#include <sqlite3.h>
#include <SDL3/SDL.h>

#include <TileReader.hpp>

bool TileReader::Open(const std::string& filename) {
    if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
        SDL_Log("Failed to open database: %s", sqlite3_errmsg(db));
        return false;
    }

    SDL_Log("OPENED TILE DATABASE");

    #ifdef _DEBUG
    sqlite3_stmt* stmt = nullptr;
    static const char* sql =
        "SELECT data "
        "FROM tiles "
        "WHERE z=? AND x=? AND y=?;";
    if (sqlite3_prepare_v2(
            db,"SELECT name FROM sqlite_master WHERE type='table';",
            -1,
            &stmt,
            nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            SDL_Log("TABLE: %s", sqlite3_column_text(stmt, 0));
        }

        sqlite3_finalize(stmt);
    }
    #endif

    return true;
}

bool TileReader::GetTile(int z, int x, int y, std::vector<uint8_t>& data) {
    static const char* sql =
        "SELECT data "
        "FROM tiles "
        "WHERE z=? AND x=? AND y=?;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        SDL_Log("SQLite prepare failed: %s", sqlite3_errmsg(db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, z);
    sqlite3_bind_int(stmt, 2, x);
    sqlite3_bind_int(stmt, 3, y);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    const auto* blob = static_cast<const uint8_t*>(sqlite3_column_blob(stmt, 0));
    int size = sqlite3_column_bytes(stmt, 0);

    data.assign(blob, blob + size);

    sqlite3_finalize(stmt);
    return true;
}

MapCenter TileReader::GetCenter() const {
    MapCenter center{};

    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT name, value "
        "FROM metadata "
        "WHERE name='center_lon' OR name='center_lat';";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return center;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* name =  reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        if (strcmp(name, "center_lon") == 0) center.lon = atof(value);
        if (strcmp(name, "center_lat") == 0) center.lat = atof(value);
    }

    sqlite3_finalize(stmt);
    return center;
}

MapBounds TileReader::GetBounds() const {
    MapBounds bounds{};

    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT name, value "
        "FROM metadata "
        "WHERE name='west' "
        "   OR name='south' "
        "   OR name='east' "
        "   OR name='north';";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return bounds;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        double d = atof(value);

        if (strcmp(name, "west") == 0)
            bounds.west = d;
        else if (strcmp(name, "south") == 0)
            bounds.south = d;
        else if (strcmp(name, "east") == 0)
            bounds.east = d;
        else if (strcmp(name, "north") == 0)
            bounds.north = d;
    }

    sqlite3_finalize(stmt);
    return bounds;
}

void TileReader::Shutdown() {
    if (db) sqlite3_close(db);
}