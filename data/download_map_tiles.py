import sqlite3
import requests
import mercantile
from tqdm import tqdm
from datetime import datetime, timezone

west  = -77.61978149
south = 38.43799313
east  = -77.23148346
north = 38.68175775

zoom = 17

output = "tiles/quantico_zoom_17.sqlite"

url_template = (
    "https://services.arcgisonline.com/ArcGIS/rest/services/"
    "World_Imagery/MapServer/tile/{z}/{y}/{x}"
)

source_url = (
    "https://services.arcgisonline.com/ArcGIS/rest/services/"
    "World_Imagery/MapServer"
)

db = sqlite3.connect(output)
cur = db.cursor()

cur.execute("""
DROP TABLE IF EXISTS tiles
""")

cur.execute("""
CREATE TABLE tiles (
    z INTEGER NOT NULL,
    x INTEGER NOT NULL,
    y INTEGER NOT NULL,
    data BLOB NOT NULL,
    PRIMARY KEY (z, x, y)
)
""")

cur.execute("""
DROP TABLE IF EXISTS metadata
""")

cur.execute("""
CREATE TABLE metadata (
    name TEXT PRIMARY KEY,
    value TEXT
)
""")

tiles = list(mercantile.tiles(west, south, east, north, [zoom]))
print(f"TILES TO DOWNLOAD: {len(tiles)}")

min_x = min(t.x for t in tiles)
max_x = max(t.x for t in tiles)

min_y = min(t.y for t in tiles)
max_y = max(t.y for t in tiles)

session = requests.Session()

downloaded = 0
failed = 0

for tile in tqdm(tiles):

    url = url_template.format(
        z=tile.z,
        x=tile.x,
        y=tile.y
    )

    try:
        response = session.get(url, timeout = 20)
    except Exception as e:
        print("Request failed:", tile, e)
        failed += 1
        continue

    if response.status_code != 200:
        print("HTTP failed:", tile, response.status_code)
        failed += 1
        continue

    cur.execute(
        """
        INSERT OR REPLACE INTO tiles
        (z, x, y, data)
        VALUES (?, ?, ?, ?)
        """,
        (tile.z, tile.x, tile.y, sqlite3.Binary(response.content))
    )

    downloaded += 1


db.commit()

print()
print("DOWNLOADED:", downloaded)
print("FAILED:", failed)

center_lon = (west + east) / 2.0
center_lat = (south + north) / 2.0

metadata = [
    ("name", "Quantico World Imagery"),
    ("description", "ArcGIS World Imagery XYZ tile cache"),

    ("scheme", "xyz"),
    ("projection", "EPSG:3857"),
    ("tile_size", "256"),

    ("format", "jpg"),

    ("min_zoom", str(zoom)),
    ("max_zoom", str(zoom)),
    ("center_zoom", str(zoom)),

    ("west", str(west)),
    ("south", str(south)),
    ("east", str(east)),
    ("north", str(north)),

    ("center_lon", str(center_lon)),
    ("center_lat", str(center_lat)),

    ("min_x", str(min_x)),
    ("max_x", str(max_x)),
    ("min_y", str(min_y)),
    ("max_y", str(max_y)),

    ("tile_count_requested", str(len(tiles))),
    ("tile_count_downloaded", str(downloaded)),
    ("tile_count_failed", str(failed)),

    ("source", source_url),
    ("created", datetime.now(timezone.utc).isoformat())
]

cur.executemany(
    """
    INSERT OR REPLACE INTO metadata
    (name, value)
    VALUES (?, ?)
    """,
    metadata
)

db.commit()
db.close()

print()
print("CREATED:", output)