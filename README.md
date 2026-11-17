# Embedded space scene

A FreeGLUT/OpenGL scene with embedded ship and Adrian meshes, two texture maps, a saved camera path, and a particle field.

## Build and run

Requires a C++17 compiler, Make, and the FreeGLUT, GLEW, OpenGL, and GLU development libraries.

```sh
make
make run
```

The executable is `build/phm-scene`. Open this repository folder in VS Code to use **Build scene** (Ctrl+Shift+B), **Run scene** from Tasks, or **Build and Debug Scene** (F5). The VS Code tasks compile and link all files in `src/`.

## Controls

- Hold **W** or **S** to move between the saved positions.
- Press **3** or click **Play** to animate from pos1 to pos2.
- Press **Esc** to exit.

## Layout

- `src/`: application, camera, particles, rendering, mesh setup, and texture loading.
- `src/generated/`: embedded mesh arrays used by the executable.
- `include/`: shared scene and mesh types.
- `assets/camera/`: `pos1.csv` and `pos2.csv`.
- `assets/textures/`: Adrian's texture maps.
- `third_party/`: the `stb_image` image loader.
- `misc/`: original OBJ models and the OpenGL syntax reference, kept for reference but not needed to run.

The program resolves `assets/` relative to the executable in `build/`, so it can be launched from any working directory while the repository layout stays intact.
