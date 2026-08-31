# Map-Viewer

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
