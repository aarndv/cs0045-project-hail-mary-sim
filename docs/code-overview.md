# Code and Controls Overview

This document summarizes the controls and main parts of `main.cpp` for *The Petrova Line Above Adrian*.

## Keyboard and mouse controls

| Input | Action |
| --- | --- |
| Hold **W** | Move forward along the Camera Tour. |
| Hold **S** | Move backward along the Camera Tour. |
| **3** | Start or replay the Camera Tour. |
| **4** | Enter orbit mode around Adrian. |
| **Left mouse drag** in orbit mode | Move the camera around Adrian; vertical dragging changes its elevation. The camera stays aimed at the planet. |
| **Esc** | Exit the application. |

## Important functions and callbacks

| Function | Purpose |
| --- | --- |
| `main()` | Initializes the window, OpenGL state, meshes, textures, and callbacks. |
| `display()` | Renders the stars, ship, Adrian, particles, and text overlay. |
| `reshape()` | Updates the viewport and perspective projection when the window changes size. |
| `applyPathPosition()` | Updates camera position and orientation along the Camera Tour. |
| `updateOrbitCamera()` | Places the camera around Adrian at a fixed distance and aims it at the planet. |
| `drawParticles()` | Renders the animated crimson particle field. |
| `drawStars()` | Draws the 250 distant background stars. |
| `drawHud()` | Renders the text overlay in screen coordinates. |
| `keyboardDown()` | Handles key presses for movement, Camera Tour, orbit mode, and exit. |
| `keyboardUp()` | Stops movement when W or S is released. |
| `mouseButton()` | Tracks left mouse button press and release in orbit mode. |
| `mouseDrag()` | Updates orbit angle and elevation while the mouse is dragged. |
| `timer()` | Advances the Camera Tour and schedules regular updates. |

## Important variables

| Variable | Purpose |
| --- | --- |
| `camera` | Current camera position in 3D space. |
| `cameraYaw`, `cameraPitch` | Direction the camera faces. |
| `pathPosition` | Camera Tour progress, from 0 at the start to 1 at the destination. |
| `orbitingAdrian` | Whether Adrian orbit mode is active. |
| `orbitAngle`, `orbitElevation` | Camera position around Adrian. |
| `draggingOrbit` | Whether a left mouse drag is in progress. |
| `particles` | Fixed-size array containing the particle field. |
| `modelMesh`, `adrianMesh` | GPU buffer handles for the ship and planet meshes. |
| `adrianTexture`, `redAdrianTexture` | Planet textures selected according to tour progress. |

## Graphics features

| Feature | Implementation |
| --- | --- |
| **3D objects** | The ship and Adrian are indexed triangle meshes; particle disks are triangle fans. |
| **2D element** | The text overlay uses an orthographic projection. |
| **Translation** | `glTranslatef()` shifts the ship; planet vertices are placed in scene coordinates. |
| **Rotation** | Camera yaw and pitch set the view direction; orbit angle and elevation move the camera around Adrian. |
| **Scaling** | Adrian's vertices are scaled around the mesh's original center during setup. |
| **Lighting** | `GL_LIGHT2` gives the ship colored bounce light; `GL_LIGHT0` illuminates Adrian. |
| **Texture mapping** | Image textures are loaded and bound, then mesh UV coordinates map them onto Adrian. |
| **VBO setup** | `uploadMesh()` creates and fills vertex and index buffers using `glGenBuffers()` and `glBufferData()`. |
| **VBO rendering** | `drawMesh()` binds those buffers, sets vertex pointers, and calls `glDrawElements()`. |

The data headers in `data/` contain the ship mesh, Adrian mesh, and star positions. Texture images live in `assets/textures/`.
