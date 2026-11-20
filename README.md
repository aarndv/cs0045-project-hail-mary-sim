# The Petrova Line Above Adrian

An interactive FreeGLUT/OpenGL recreation inspired by *Project Hail Mary*. In Andy Weir's story, science teacher Ryland Grace wakes aboard a spacecraft far from Earth and must find why stars are losing energy. His investigation leads to Astrophage, a microscopic organism that feeds on stellar energy. [Sony Pictures' synopsis](https://www.sonypictures.co.uk/movies/project-hail-mary) describes Grace's mission and the mystery he must solve; the [publisher's description](https://www.penguin.co.uk/books/440237/project-hail-mary-by-weir-andy/9781807840020) frames it as a last-chance mission to save Earth.

This scene focuses on the Petrova Line above Adrian. Ryland Grace and his companion Rocky collect crimson Astrophage and discover microbial life in Adrian's atmosphere. That discovery may offer a way to protect both of their suns. The recreation includes the ship, Adrian, 250 generated background stars, animated particles, a camera tour, and a mouse-controlled orbit view.

## Build and run

### Windows (recommended)

Use [MSYS2](https://www.msys2.org/) with its **UCRT64** environment. In the **MSYS2 UCRT64** terminal, install the compiler, debugger, and graphics libraries:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-freeglut mingw-w64-ucrt-x86_64-glew
```

Then open the repository folder in VS Code. The checked-in tasks use `C:\\msys64\\ucrt64\\bin` by default. Use **Build scene** (Ctrl+Shift+B), **Run scene** from Tasks, or **Build and Debug Scene** (F5). If MSYS2 is installed elsewhere, update the paths in `.vscode/tasks.json`, `.vscode/launch.json`, and `.vscode/c_cpp_properties.json`.

To build and run directly from the UCRT64 terminal:

```sh
g++ -g -std=c++17 -Wall -Wextra main.cpp -o phm-scene.exe -lfreeglut -lopengl32 -lglu32 -lglew32
./phm-scene.exe
```

### Linux

Requires a C++17 compiler plus FreeGLUT, GLEW, OpenGL, and GLU development libraries. On Debian or Ubuntu, install them with:

```sh
sudo apt install g++ freeglut3-dev libglew-dev libglu1-mesa-dev
```

```sh
g++ -g -std=c++17 -Wall -Wextra main.cpp -o phm-scene -lglut -lGL -lGLU -lGLEW
./phm-scene
```

The VS Code build and debug tasks also retain Linux settings for users building there.

## Controls

- Hold **W** or **S** to move forward or backward along the camera path.
- Press **3** to start or replay the **Camera Tour**.
- Press **4** to orbit around Adrian.
- In orbit mode, hold the **left mouse button** and drag to change the viewing angle and elevation.
- Press **Esc** to exit.

## Layout

- `main.cpp`: camera, mesh setup, textures, particles, HUD, and input, grouped into sections.
- `data/mary_data.h` and `data/adrian_data.h`: embedded ship and planet mesh data used by `main.cpp`.
- `data/stars_data.h`: 250 star positions, colors, brightness levels, and point sizes.
- `data/stb_image.h`: the single-header image decoder used by `loadTexture()` for JPG and PNG textures.
- Camera positions and angles are hardcoded in `main.cpp`.
- `assets/textures/`: Adrian's texture maps.

The program resolves `assets/` relative to the executable, so it can be launched from any working directory while the repository layout stays intact.

For a feature and controls summary, see [Code and Controls Overview](docs/code-overview.md).
For the five-person walkthrough, see [Video Presentation Script](docs/video-presentation-script.md).
