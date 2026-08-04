import os
import time
import sqlite3
import requests
import mercantile
from tqdm import tqdm
from pathlib import Path
from datetime import datetime, timezone
from requests.adapters import HTTPAdapter
from urllib3.util.retry import Retry

output = "./data/tiles/quantico_hybrid_map.sqlite"

output_path = Path(output)
output_path.parent.mkdir(parents=True, exist_ok=True)

API_KEY = os.environ["MAPTILER_API_KEY"]

url_template = (
    "https://api.maptiler.com/maps/hybrid-v4/"
    "{z}/{x}/{y}.jpg?key=" + API_KEY
)
source_url = "https://api.maptiler.com/maps/hybrid-v4"

db = sqlite3.connect(output_path)
cur = db.cursor()

cur.execute("""
CREATE TABLE IF NOT EXISTS tiles (
    z INTEGER NOT NULL,
    x INTEGER NOT NULL,
    y INTEGER NOT NULL,
    data BLOB NOT NULL,
    PRIMARY KEY (z, x, y)
)
""")

cur.execute("""
CREATE TABLE IF NOT EXISTS metadata (
    name TEXT PRIMARY KEY,
    value TEXT
)
""")

db.commit()

west  = -77.61978149
south = 38.43799313
east  = -77.23148346
north = 38.68175775

min_zoom = 14
max_zoom = 17

tiles = []

for zoom in range(min_zoom, max_zoom + 1):
    tiles.extend(mercantile.tiles(west, south, east, north, [zoom]))


print("TOTAL REQUESTED:", len(tiles))

cur.execute("SELECT z, x, y FROM tiles")
existing = set(cur.fetchall())
tiles = [
    t for t in tiles
    if (t.z, t.x, t.y) not in existing
]

print("ALREADY CACHED:", len(existing))
print("REMAINING:", len(tiles))

if not tiles:
    print("Nothing to download.")
    db.close()
    exit()

def new_session():
    retry = Retry(
        total = 5,
        connect = 5,
        read = 5,
        backoff_factor = 2,
        status_forcelist=[429, 500, 502, 503, 504 ],
        allowed_methods=["GET"],
    )
    adapter = HTTPAdapter(
        max_retries=retry,
        pool_connections = 10,
        pool_maxsize = 10,
    )
    s = requests.Session()
    s.mount("https://", adapter)
    s.headers.update({ "User-Agent": "TileDownloader/1.0" })

    return s

session = new_session()

downloaded = 0
failed = []

start_time = datetime.now(timezone.utc)
delay = 0.1

progress = tqdm(
    tiles,
    desc="DOWNLOADING",
    unit="TILE"
)

for tile in progress:

    url = url_template.format(z=tile.z, x=tile.x, y=tile.y)
    success = False

    for attempt in range(6):

        try:
            response = session.get(url, timeout=(5, 60))

            if response.status_code == 200:
                success = True
                delay = max(0.1, delay * 0.95)
                break

            elif response.status_code == 403:
                wait = min(60, 2 ** attempt)
                print(f"403 {tile}, WAITING {wait} SECONDS")
                time.sleep(wait)

            else:
                print("HTTP ERROR:", response.status_code, tile)
                break


        except (
            requests.exceptions.ConnectionError,
            requests.exceptions.ReadTimeout,
            requests.exceptions.ConnectTimeout,
        ) as e:
            wait = min(60, 2 ** attempt)

            print(f"{tile} FAILED: {e}")
            print(f"RETRYING IN {wait} SECONDS")

            session.close()
            session = new_session()

            time.sleep(wait)


    if not success:
        failed.append(tile)
        continue


    cur.execute(
        """
        INSERT OR REPLACE INTO tiles
        (z, x, y, data)
        VALUES (?, ?, ?, ?)
        """,
        (
            tile.z, tile.x, tile.y,
            sqlite3.Binary(response.content)
        )
    )

    downloaded += 1

    elapsed = (datetime.now(timezone.utc) - start_time).total_seconds()
    rate = downloaded / elapsed if elapsed > 0 else 0
    progress.set_postfix(
        DOWNLOADED=downloaded,
        FAILED=len(failed)
    )

    if downloaded % 250 == 0:
        db.commit()

    time.sleep(delay)

db.commit()

print()
print("NEW DOWNLOADS:", downloaded)
print("FAILED:", len(failed))

center_lon = (west + east) / 2
center_lat = (south + north) / 2

metadata = [
    ("name", "Quantico Hybrid"),

    ("description",
     "MapTiler Hybrid raster tile cache"),

    ("scheme", "xyz"),
    ("projection", "EPSG:3857"),
    ("tile_size", "256"),
    ("format", "jpg"),

    ("min_zoom", str(min_zoom)),
    ("max_zoom", str(max_zoom)),

    ("west", str(west)),
    ("south", str(south)),
    ("east", str(east)),
    ("north", str(north)),

    ("center_lon", str(center_lon)),
    ("center_lat", str(center_lat)),

    ("tile_count_requested", str(len(tiles))),
    ("tile_count_downloaded", str(downloaded)),
    ("tile_count_failed", str(len(failed))),

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