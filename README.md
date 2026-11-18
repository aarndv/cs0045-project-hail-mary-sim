# The Petrova Line Above Adrian

An interactive FreeGLUT/OpenGL recreation inspired by *Project Hail Mary*. In Andy Weir's story, science teacher Ryland Grace wakes aboard a spacecraft far from Earth and must find why stars are losing energy. His investigation leads to Astrophage, a microscopic organism that feeds on stellar energy. [Sony Pictures' synopsis](https://www.sonypictures.co.uk/movies/project-hail-mary) describes Grace's mission and the mystery he must solve; the [publisher's description](https://www.penguin.co.uk/books/440237/project-hail-mary-by-weir-andy/9781807840020) frames it as a last-chance mission to save Earth.

This scene focuses on the Petrova Line above Adrian. Ryland Grace and his companion Rocky collect crimson Astrophage and discover microbial life in Adrian's atmosphere. That discovery may offer a way to protect both of their suns. The recreation includes the ship, Adrian, 250 generated background stars, animated particles, a camera tour, and a mouse-controlled orbit view.

## Build and run

Requires a C++17 compiler plus the FreeGLUT, GLEW, OpenGL, and GLU development libraries.

```sh
g++ -g -std=c++17 main.cpp -o phm-scene -lglut -lGL -lGLU -lGLEW
./phm-scene
```

Open this repository folder in VS Code to use **Build scene** (Ctrl+Shift+B), **Run scene** from Tasks, or **Build and Debug Scene** (F5). The build task uses the same single-file `g++` command.

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
