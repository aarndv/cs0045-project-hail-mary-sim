# Video Presentation Script

**Target length:** about 4 minutes 30 seconds  
**Presenters:** five members; four explain the code, and one demonstrates the output.

The first 2 minutes 40 seconds show the source code. The remaining time shows the running scene. The linked lines are screen cues, not lines to read aloud.

## 0:00–0:35 — Member 1: concept and structure

**Show:** [Scene state](../main.cpp#L83), then [`display()`](../main.cpp#L574).

**Say:** “Our scene is inspired by *Project Hail Mary*. It recreates a view above Adrian, with the ship, planet, distant stars, and a glowing particle field representing the Petrova Line. The main scene variables track the camera and objects. `display()` brings those parts together each frame.”

## 0:35–1:20 — Member 2: camera and controls

**Show:** [`applyPathPosition()`](../main.cpp#L137), [`updateOrbitCamera()`](../main.cpp#L202), [`keyboardDown()`](../main.cpp#L659), and [`mouseDrag()`](../main.cpp#L699).

**Say:** “There are two ways to explore the scene. The Camera Tour interpolates between hardcoded camera positions. Orbit mode places the camera at a fixed distance from Adrian and keeps it facing the planet. Keys 3 and 4 switch modes, while mouse dragging changes the orbit angle and elevation.”

## 1:20–2:00 — Member 3: meshes and appearance

**Show:** [`uploadMesh()`](../main.cpp#L221), [`drawMesh()`](../main.cpp#L486), [`loadTexture()`](../main.cpp#L265), and the [lighting setup](../main.cpp#L602).

**Say:** “The ship and Adrian are triangle meshes. `uploadMesh()` puts their vertices and indices in GPU buffers, and `drawMesh()` uses those buffers to render them. Adrian receives image textures through UV coordinates. Separate light settings give the ship a colored reflection and illuminate the planet.”

## 2:00–2:40 — Member 4: particles, stars, and text

**Show:** [`initializeParticles()`](../main.cpp#L295), [`drawParticles()`](../main.cpp#L380), [`drawStars()`](../main.cpp#L464), and [`drawHud()`](../main.cpp#L520).

**Say:** “The particle functions create and draw the glowing crimson field as the tour progresses. `drawStars()` adds 250 distant stars with different brightness and sizes. `drawHud()` places the scene description, current camera status, and controls over the 3D view.”

## 2:40–4:30 — Member 5: output demonstration

**Show:** Run the application at its normal 1920×1080 window size. Move slowly enough for each visual change to be clear.

**Say:** “Here is the finished scene. First, I’ll hold W and S to move forward and backward. Next, I’ll press 3 to play the Camera Tour and show how the atmosphere and lighting change as the particle field appears. Now I’ll press 4 to view Adrian from orbit. The camera stays still until I drag with the left mouse button; I can move around the planet while it stays centered at the same distance. The stars and text guide remain visible in both views.”

**Demonstration order:**

1. Show the starting scene and briefly point out the ship, Adrian, stars, and description.
2. Hold W, then S, to show manual movement.
3. Press 3 and let the Camera Tour reach the glowing crimson field.
4. Press 4 to switch to Adrian orbit mode.
5. Drag left, right, up, and down to show the fixed-distance camera orbit.
