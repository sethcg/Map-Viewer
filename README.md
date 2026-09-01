# Map-Viewer

### Description:
<dl>
  <dd>
    <dl>
      <dd>
        Offline satellite map viewer. Map image tiles are downloaded with a script and compressed into a SQLite database. The application uses these images for viewing through the use of mouse movements, increasing the quality of image as zoom is applied. There are some additional debug features, such as boundary information and a tile border option.
      </dd>
    </dl>
  </dd>
</dl>

### Preview:

<dl>
  <dd>
    <details closed>
    <summary>Demo Video</summary>
      <video src="https://github.com/user-attachments/assets/05241983-cfe7-4e72-8ca9-45ac8902e56f" style="max-width: 730px;" />
    </details>
    <details open>
    <summary>Images</summary>
      <img width="1278" height="756" alt="Map-Viewer-TileBorder" src="https://github.com/user-attachments/assets/8f58811f-5c14-4d66-a9f3-cee61de01c83" />
      <img width="1285" height="762" alt="Map-Viewer-No-TileBorder" src="https://github.com/user-attachments/assets/d47b5be7-390f-4f96-978a-02e18e0dfd02" />
    </details>
  </dd>
</dl>

### Tooling:


  - [x] SDL3
  - [x] SQLite
  - [x] OpenGL
  - [x] ImGui
  - [x] Vcpkg
  - [x] CMake

### Developer Notes:

```bash

# CLONE REPOSITORY (WITH SUBMODULE)
git clone --recursive https://github.com/sethcg/Map-Viewer.git

# CONFIGURE
cmake --preset windows-vs2026

# BUILD RELEASE (INCREASED PERFORMANCE)
cmake --build --preset windows-vs2026-release

# BUILD DEBUG
cmake --build --preset windows-vs2026-debug

```

<details closed>
<summary><b>Line Count</b></summary>
<br/>

```bash

# GET LINE COUNT (REQUIRES CLOC TO BE INSTALLED)
cloc --include-lang=C++,"C/C++ Header",CMake,GLSL `
  --force-lang=GLSL,.frag `
  --force-lang=GLSL,.vert `
  --exclude-dir=build,vcpkg `
  --out=line-count.txt .

```

| Language         | Files  | Blank | Comment | Code   |
|:-----------------|:------:|:-----:|:-------:|:------:|
| **C++**          | 11     | 381   | 61      | 1426   |
| **C/C++ Header** | 10     | 174   | 7       | 407    |
| **CMake**        | 3      | 20    | 11      | 66     |
| **GLSL**         | 8      | 30    | 0       | 68     |
|                                                      |
| **Total**        | 32     | 605   | 79      | 1967   |

</details>
