import os
import sqlite3
import time
from datetime import datetime, timezone
from pathlib import Path

import mercantile
import requests
from requests.adapters import HTTPAdapter
from tqdm import tqdm
from urllib3.util.retry import Retry

API_KEY = os.environ["MAPTILER_API_KEY"]
MAP_FILENAME = os.environ["MAP_FILENAME"]

NORTH = float(os.environ["MAP_NORTH"])
SOUTH = float(os.environ["MAP_SOUTH"])
EAST = float(os.environ["MAP_EAST"])
WEST = float(os.environ["MAP_WEST"])

MIN_ZOOM = int(os.environ["MAP_MIN_ZOOM"])
MAX_ZOOM = int(os.environ["MAP_MAX_ZOOM"])

OUTPUT_PATH = Path("data/tiles") / f"{MAP_FILENAME}.sqlite"

URL_TEMPLATE = (
    "https://api.maptiler.com/maps/hybrid-v4/"
    "{z}/{x}/{y}.jpg"
)

SOURCE_URL = "https://api.maptiler.com/maps/hybrid-v4"

def validate_config():
    if not -90 <= SOUTH <= 90:
        raise ValueError(f"INVALID SOUTH: {SOUTH}")
    if not -90 <= NORTH <= 90:
        raise ValueError(f"INVALID NORTH: {NORTH}")
    if not -180 <= WEST <= 180:
        raise ValueError(f"INVALID WEST: {WEST}")
    if not -180 <= EAST <= 180:
        raise ValueError(f"INVALID EAST: {EAST}")
    if SOUTH > NORTH:
        raise ValueError("SOUTH MUST BE LESS THAN NORTH")
    if MIN_ZOOM < 0:
        raise ValueError("MIN_ZOOM MUST BE GREATER THAN ZERO")
    if MAX_ZOOM < MIN_ZOOM:
        raise ValueError("MAX_ZOOM MUST BE GREATER THAN MIN_ZOOM")


def create_session():
    retry = Retry(
        total = 5,
        connect = 5,
        read = 5,
        backoff_factor = 1,
        status_forcelist = (429, 500, 502, 503, 504),
        allowed_methods = ("GET"),
        respect_retry_after_header = True,
    )

    adapter = HTTPAdapter(
        max_retries=retry,
        pool_connections = 10,
        pool_maxsize = 10,
    )

    session = requests.Session()
    session.mount("https://", adapter)
    session.headers["User-Agent"] = "TileDownloader/1.0"

    return session


def create_database(path):
    path.parent.mkdir(parents = True, exist_ok = True)

    db = sqlite3.connect(path)
    db.execute("PRAGMA journal_mode=WAL")
    db.execute("PRAGMA synchronous=NORMAL")
    db.execute("""
        CREATE TABLE IF NOT EXISTS tiles (
            z INTEGER NOT NULL,
            x INTEGER NOT NULL,
            y INTEGER NOT NULL,
            data BLOB NOT NULL,
            PRIMARY KEY (z, x, y)
        )
    """)
    db.execute("""
        CREATE TABLE IF NOT EXISTS metadata (
            name TEXT PRIMARY KEY,
            value TEXT
        )
    """)
    db.commit()

    return db


def get_tiles():
    for zoom in range(MIN_ZOOM, MAX_ZOOM + 1):
        yield from mercantile.tiles(WEST, SOUTH, EAST, NORTH, [zoom])


def tile_exists(db, tile):
    row = db.execute(
        """
        SELECT 1
        FROM tiles
        WHERE z = ? AND x = ? AND y = ?
        """,
        (tile.z, tile.x, tile.y),
    ).fetchone()

    return row is not None


def download_tile(session, tile):
    url = (URL_TEMPLATE.format(z = tile.z, x = tile.x, y = tile.y) + f"?key={API_KEY}")

    try:
        response = session.get(url, timeout = (5, 60),)
        response.raise_for_status()

        if not response.content:
            raise RuntimeError("Empty response")

        return response.content

    except requests.RequestException as error:
        print(f"FAILED {tile}: {error}")
        return None


def write_metadata(db):
    center_lon = (WEST + EAST) / 2
    center_lat = (SOUTH + NORTH) / 2

    metadata = [
        ("name", MAP_FILENAME),
        ("scheme", "xyz"),
        ("projection", "EPSG:3857"),
        ("tile_size", "256"),
        ("format", "jpg"),
        ("min_zoom", str(MIN_ZOOM)),
        ("max_zoom", str(MAX_ZOOM)),
        ("west", str(WEST)),
        ("south", str(SOUTH)),
        ("east", str(EAST)),
        ("north", str(NORTH)),
        ("center_lon", str(center_lon)),
        ("center_lat", str(center_lat)),
        ("source", SOURCE_URL),
        ("created", datetime.now(timezone.utc).isoformat()),
    ]

    db.executemany(
        """
        INSERT OR REPLACE INTO metadata (name, value)
        VALUES (?, ?)
        """,
        metadata,
    )

    db.commit()


def main():
    validate_config()

    db = create_database(OUTPUT_PATH)
    session = create_session()

    tiles = list(get_tiles())

    print(f"TOTAL REQUESTED: {len(tiles):,}")

    remaining = [
        tile
        for tile in tiles
        if not tile_exists(db, tile)
    ]

    print(f"ALREADY CACHED: {len(tiles) - len(remaining):,}")
    print(f"REMAINING: {len(remaining):,}")

    if not remaining:
        write_metadata(db)
        db.close()

        print("NOTHING TO DOWNLOAD.")
        return

    downloaded = 0
    failed = []

    start_time = time.monotonic()

    progress = tqdm(
        remaining,
        desc="DOWNLOADING",
        unit="tile",
    )

    try:
        for tile in progress:
            data = download_tile(session, tile)

            if data is None:
                failed.append(tile)
                continue

            db.execute(
                """
                INSERT OR REPLACE INTO tiles
                (z, x, y, data)
                VALUES (?, ?, ?, ?)
                """,
                (tile.z, tile.x, tile.y, sqlite3.Binary(data)),
            )

            downloaded += 1

            if downloaded % 250 == 0:
                db.commit()

            elapsed = time.monotonic() - start_time
            rate = downloaded / elapsed if elapsed else 0

            progress.set_postfix(
                downloaded=downloaded,
                failed=len(failed),
                rate=f"{rate:.1f}/s",
            )

            time.sleep(0.1)

        db.commit()
        write_metadata(db)

    finally:
        session.close()
        db.close()

    print()
    print(f"NEW DOWNLOADS: {downloaded:,}")
    print(f"FAILED: {len(failed):,}")
    print(f"CREATED: {OUTPUT_PATH}")


if __name__ == "__main__":
    main()