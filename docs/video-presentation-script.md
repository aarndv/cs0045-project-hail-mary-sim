# Video Presentation Script: The Petrova Line Above Adrian

**Target duration:** about 4½ minutes at a measured speaking pace.

**Presenters:** Members 1–4 explain the code. Member 5 demonstrates the running output.
**Recording rule:** Members 1–4 read the presentation paragraphs aloud. Member 5 narrates the live demonstration impromptu, following the **Show** cues. The cues are not spoken. Do not read the code line by line.

## 0:00–0:55 — Member 1: scene concept and program structure

**Show:** [README scene description](../README.md#L3).

Our project recreates a scene inspired by *Project Hail Mary*. Ryland Grace explores the Petrova Line above Adrian, where the glowing Astrophage field leads to a discovery that may help save two suns. The scene includes a ship, the planet, a crimson particle field, distant stars, and two camera views. The camera first approaches the field, then gives us a closer view of Adrian.

**Show:** [Scene objects and camera variables](../main.cpp#L82), then [hardcoded camera endpoints](../main.cpp#L101).

These variables hold the two three-dimensional meshes, their textures, the camera position and viewing angles, and the tour and orbit states. The Camera Tour uses endpoints written here in the source, so key 3 can restart the same route without loading camera files.

**Show:** [`main()` and callback registration](../main.cpp#L765), then [`display()`](../main.cpp#L574).

`main()` prepares the window, meshes, textures, and controls. Every redraw goes through `display()`, which sets the camera, draws the stars and three-dimensional objects, adds the particle field, and places the text overlay on top so it stays readable.

## 0:55–1:45 — Member 2: camera movement and controls

**Show:** [`loadCameraPath()`](../main.cpp#L158), [`startAnimation()`](../main.cpp#L167), and [`applyPathPosition()`](../main.cpp#L137).

`loadCameraPath()` calculates the distance between the hardcoded endpoints. Key 3 calls `startAnimation()` to begin the tour again. `applyPathPosition()` moves the camera between the endpoints and gradually changes its yaw and pitch, turning the view as the camera travels.

**Show:** [`updateOrbitCamera()`](../main.cpp#L202), focusing on the fixed distance and aim calculations.

Key 4 enters the second view. `updateOrbitCamera()` places the camera on an imaginary sphere around Adrian and aims it back toward the planet. This keeps Adrian in view at a fixed distance as horizontal and vertical drags change the angle and elevation.

**Show:** [`keyboardDown()`](../main.cpp#L659), [`keyboardUp()`](../main.cpp#L720), [`mouseButton()`](../main.cpp#L688), [`mouseDrag()`](../main.cpp#L699), and [`timer()`](../main.cpp#L727).

The keyboard callbacks handle W, S, 3, 4, and Escape. `mouseButton()` detects a left drag, and `mouseDrag()` updates the orbit. The timer advances the tour, while orbit mode stays still until the mouse moves. The HUD reports the current state and controls.

## 1:45–2:35 — Member 3: meshes, transformations, lighting, and textures

**Show:** [`initializeSceneMeshes()`](../main.cpp#L241), especially the planet vertex calculations.

The ship and Adrian are separate three-dimensional triangle meshes. `initializeSceneMeshes()` loads them, enlarges Adrian around its center, and places it in the scene. That demonstrates scaling and translation. The ship also receives a translation when it is drawn.

**Show:** [`uploadMesh()`](../main.cpp#L221), then [`drawMesh()`](../main.cpp#L486).

`uploadMesh()` copies each mesh’s vertices and triangle indices into GPU buffers. `drawMesh()` binds those buffers and draws the triangles. Each vertex includes a position, surface normal, and texture coordinates for lighting and image mapping.

**Show:** [`loadTexture()`](../main.cpp#L265), [`drawMesh()` texture selection](../main.cpp#L488), and [ship and planet lighting](../main.cpp#L596).

`loadTexture()` prepares the images, and `drawMesh()` maps Adrian’s texture onto the planet. In `display()`, one light gives the ship a colored bounce while another lights Adrian. The ambient color becomes redder and the planet’s texture changes as the tour reaches the Astrophage field.

## 2:35–3:15 — Member 4: particles, stars, and the two-dimensional overlay

**Show:** [`initializeParticles()`](../main.cpp#L295) and [`animatedParticlePosition()`](../main.cpp#L344).

`initializeParticles()` gives the crimson particles different positions, sizes, colors, and appearance times. `animatedParticlePosition()` updates their positions as the tour progresses, so the field emerges gradually as the camera approaches.

**Show:** [`drawParticleDisk()`](../main.cpp#L361) and [`drawParticles()`](../main.cpp#L380).

`drawParticleDisk()` draws each particle as a camera-facing triangle fan with a soft, transparent edge. `drawParticles()` blends visible particles into a glow. Together, the ship, Adrian, and particle disks provide the three geometric objects in the scene.

**Show:** [`data/stars_data.h` star count](../data/stars_data.h#L16), [`drawStars()`](../main.cpp#L464), and [`drawHud()`](../main.cpp#L520).

The star data file contains 250 background stars with varied positions, brightness, and sizes. `drawStars()` renders them behind the objects. `drawHud()` switches to screen coordinates to show the camera status, controls, and scene description over the three-dimensional view.

## 3:15–4:30 — Member 5: live output demonstration

Narrate the live demonstration impromptu, using the cues below to guide the sequence.

**Show:** Start the program at its 1920 by 1080 window size. Point to the ship, Adrian, background stars, and description. Do not show source code again.

**Show:** Hold W briefly, then S. Keep the camera tour in view.

**Show:** Press 3 and let the tour progress until the glowing field appears.

**Show:** Press 4. Pause without dragging, then hold the left mouse button and drag horizontally and vertically.

**Show:** Pause on a clear view of Adrian with stars behind it, then press Esc.

## Recording checklist

- Keep the source text large enough to read while showing the linked lines.
- Use the code cues for Members 1–4 and the application output for Member 5.
- Rehearse Members 1–4 at a steady pace; keep the live demonstration concise to stay close to four and a half minutes overall.
