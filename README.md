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
| **C++**          | 12     | 426   | 80      | 1533   |
| **C/C++ Header** | 12     | 207   | 9       | 495    |
| **CMake**        | 3      | 20    | 11      | 68     |
| **GLSL**         | 8      | 30    | 0       | 68     |
|                                                      |
| **Total**        | 35     | 683   | 100     | 2164   |

</details>
