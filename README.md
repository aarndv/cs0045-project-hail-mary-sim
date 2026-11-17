# Embedded space scene

A FreeGLUT/OpenGL scene with an embedded ship and planet, two texture maps, a saved camera path, and a particle field. The application code lives in one `main.cpp`; the large generated mesh arrays and the third-party image loader remain in headers.

## Build and run

Requires a C++17 compiler plus the FreeGLUT, GLEW, OpenGL, and GLU development libraries.

```sh
g++ -g -std=c++17 main.cpp -o phm-scene -lglut -lGL -lGLU -lGLEW
./phm-scene
```

Open this repository folder in VS Code to use **Build scene** (Ctrl+Shift+B), **Run scene** from Tasks, or **Build and Debug Scene** (F5). The build task uses the same single-file `g++` command.

## Controls

- Hold **W** or **S** to move between the saved positions.
- Press **3** or click **Play** to animate from pos1 to pos2.
- Press **Esc** to exit.

## Layout

- `main.cpp`: camera, mesh setup, textures, particles, HUD, and input, grouped into sections.
- `generated/`: embedded mesh arrays used by `main.cpp`.
- `assets/camera/`: `pos1.csv` and `pos2.csv`.
- `assets/textures/`: Adrian's texture maps.
- `third_party/`: the `stb_image` image loader.
- `misc/`: original OBJ models and the OpenGL syntax reference, kept for reference but not needed to run.

The program resolves `assets/` relative to the executable, so it can be launched from any working directory while the repository layout stays intact.
