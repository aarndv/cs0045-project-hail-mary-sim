# Video Presentation Script: The Petrova Line Above Adrian

**Target duration:** about 6 minutes at a measured speaking pace.

**Presenters:** Members 1–4 explain the code. Member 5 demonstrates the running output.
**Recording rule:** Read the presentation paragraphs aloud. The **Show** cues tell the screen operator which code line or output to display; they are not spoken. Move to the next cue as the speaker begins its matching paragraph. Do not read the code line by line.

## 0:00–1:05 — Member 1: scene concept and program structure

**Show:** [README scene description](../README.md#L3).

Our project recreates a scene inspired by *Project Hail Mary*. Ryland Grace explores the Petrova Line above Adrian, where the glowing Astrophage field leads to a discovery that may help save two suns. We wanted the output to feel like a place the viewer can explore, so it has a ship, the planet, a crimson particle field, distant stars, and two camera views. The scene is arranged so the camera first approaches the planet and field, then gives the viewer a closer way to inspect Adrian. That change in viewpoint helps the demonstration feel like a short journey instead of a single static image.

**Show:** [Scene objects and camera variables](../main.cpp#L82), then [hardcoded camera endpoints](../main.cpp#L101).

These variables hold the two three-dimensional meshes, their textures, the current camera position and viewing angles, and the tour and orbit states. The Camera Tour starts and ends at the positions written here in the source, so it does not read camera files. Keeping the endpoints together in the program makes the route predictable: each run follows the same approach, and key 3 can restart it from the same opening view. The separate state values also let the program distinguish automatic playback from the manually controlled orbit view.

**Show:** [`main()` and callback registration](../main.cpp#L765), then [`display()`](../main.cpp#L574).

`main()` prepares the window, meshes, textures, and controls. Every redraw goes through `display()`, which sets the camera, draws the background stars and three-dimensional objects, adds the particle field, and finally places the text overlay on top. This order matters: the stars sit behind the scene, the meshes establish the main shapes, and the two-dimensional text remains readable over the finished view. The callbacks connect the keys and mouse to that rendering loop, so input changes the next frame rather than drawing the scene directly.

## 1:05–2:05 — Member 2: camera movement and controls

**Show:** [`loadCameraPath()`](../main.cpp#L158), [`startAnimation()`](../main.cpp#L167), and [`applyPathPosition()`](../main.cpp#L137).

`loadCameraPath()` calculates the distance between the hardcoded endpoints. When key 3 calls `startAnimation()`, the tour begins again at the start. `applyPathPosition()` moves the camera between those endpoints and gradually changes its yaw and pitch. Those angle changes are the camera rotation used during the tour. Position and viewing direction are updated together, which lets the camera travel forward while also turning to keep the important parts of the scene in view. The route position is based on elapsed time, so playback is tied to the animation clock rather than to how quickly individual frames happen to render.

**Show:** [`updateOrbitCamera()`](../main.cpp#L202), focusing on the fixed distance and aim calculations.

Key 4 enters the second view. `updateOrbitCamera()` places the camera on an imaginary sphere around Adrian and calculates the direction back toward the planet. This keeps Adrian in view and keeps the camera at the same distance while its angle or elevation changes. The horizontal mouse movement changes the orbit angle, while vertical movement changes the elevation. Because the target stays at Adrian’s center, the viewer can look from different sides without the camera drifting away from the planet.

**Show:** [`keyboardDown()`](../main.cpp#L659), [`keyboardUp()`](../main.cpp#L720), [`mouseButton()`](../main.cpp#L688), [`mouseDrag()`](../main.cpp#L699), and [`timer()`](../main.cpp#L727).

The keyboard callbacks handle W, S, 3, 4, and Escape. `mouseButton()` detects when a left drag starts or ends, and `mouseDrag()` updates the orbit angle and elevation. The timer advances the tour according to elapsed time. Orbit mode stays still until the mouse is dragged. W and S provide a direct way to adjust the camera position during the path, while the HUD reports the current state and control hints. Together, these controls make it easy to compare a guided camera move with direct exploration.

## 2:05–3:05 — Member 3: meshes, transformations, lighting, and textures

**Show:** [`initializeSceneMeshes()`](../main.cpp#L241), especially the planet vertex calculations.

The ship and Adrian are separate three-dimensional triangle meshes. `initializeSceneMeshes()` loads them, enlarges Adrian around its own center, and places it in the scene. That demonstrates scaling and translation. The ship also receives a translation when it is drawn. The planet is scaled before it is placed so its position remains visually related to the ship. These transforms let the program reuse model geometry while controlling the size and location of each object in the scene.

**Show:** [`uploadMesh()`](../main.cpp#L221), then [`drawMesh()`](../main.cpp#L486).

For the VBO requirement, `uploadMesh()` copies each mesh’s vertex data and triangle indices into GPU buffers. `drawMesh()` binds those buffers and draws the indexed triangles. The vertex data includes positions, surface normals, and texture coordinates, so the same mesh data supports shape, lighting, and texture placement. The index list lets many triangles refer back to shared vertices instead of repeating every point. Surface normals tell the lighting calculation which way each part of the surface faces, while texture coordinates map points on the mesh to locations in an image.

**Show:** [`loadTexture()`](../main.cpp#L265), [`drawMesh()` texture selection](../main.cpp#L488), and [ship and planet lighting](../main.cpp#L596).

`loadTexture()` prepares the image textures. `drawMesh()` maps one of Adrian’s textures onto the planet using its texture coordinates. In `display()`, one light gives the ship a colored bounce, while another lights Adrian. The ambient color also becomes redder as the tour reaches the Astrophage field. The planet texture changes partway through the path, matching the point where the red field becomes prominent. This transition ties the camera’s progress to a visible change in both the surface and the overall mood of the scene.

## 3:05–4:00 — Member 4: particles, stars, and the two-dimensional overlay

**Show:** [`initializeParticles()`](../main.cpp#L295) and [`animatedParticlePosition()`](../main.cpp#L344).

`initializeParticles()` gives the crimson particles different positions, sizes, colors, and appearance times. `animatedParticlePosition()` calculates where each particle belongs as the tour progresses. This produces the moving field that appears later in the scene. The positions are generated deterministically, so the field has a stable arrangement between runs. Particles do not all appear at once: their visibility grows with the tour, making the field emerge gradually as the camera approaches it.

**Show:** [`drawParticleDisk()`](../main.cpp#L361) and [`drawParticles()`](../main.cpp#L380).

Each particle is drawn by `drawParticleDisk()` as a small camera-facing triangle fan with a soft, transparent edge. `drawParticles()` checks which particles are visible and blends them into a glow. Together, the ship, Adrian, and particle disks provide the three geometric objects in the scene. The particles face the camera so their disk shapes remain visible as the viewpoint turns. Blending lets overlapping particles build up into brighter patches, which gives the field a luminous appearance without hiding the planet and ship.

**Show:** [`data/stars_data.h` star count](../data/stars_data.h#L16), [`drawStars()`](../main.cpp#L464), and [`drawHud()`](../main.cpp#L520).

The separate star data file contains 250 background stars with varied positions, brightness, and sizes. `drawStars()` renders them behind the main objects. `drawHud()` changes to screen coordinates and draws the camera status, controls, and scene description as a two-dimensional text element over the three-dimensional view. Keeping the stars in a separate data file makes their scene data easy to locate, while the overlay gives viewers context without interrupting the image. Its status text also changes with the camera mode, so the instructions stay relevant during the demo.

## 4:00–6:00 — Member 5: live output demonstration

**Show:** Start the program at its 1920 by 1080 window size. Point to the ship, Adrian, background stars, and description. Do not show source code again.

Now I’ll show the finished output. The ship and Adrian are the main objects, while the scattered stars give the background depth. The text in the corners explains the scene and shows the current control hints. This is our interpretation of the Petrova Line above Adrian. Take a moment to notice how the ship’s position gives us a sense of scale next to the planet, and how the stars help the dark background read as open space rather than an empty screen.

**Show:** Hold W briefly, then S. Keep the camera tour in view.

Holding W moves the camera forward, and holding S moves it backward. These controls change our position along the tour without moving the planet itself. Watch the status text as the camera moves; it confirms the direction of travel. The geometry stays in the scene while our viewpoint changes, which is why the planet appears to shift on screen.

**Show:** Press 3 and let the tour progress until the glowing field appears.

Pressing 3 starts the automatic Camera Tour. As the view advances, the lighting shifts toward red, Adrian’s texture changes, and the glowing particles appear. The camera, lighting, textures, and particle animation work together to tell the story of the Astrophage discovery. Notice that these effects build over the route rather than switching on all at once. That gradual change gives the viewer time to see the planet first, then recognize the field as the camera reaches it.

**Show:** Press 4. Pause without dragging, then hold the left mouse button and drag horizontally and vertically.

Pressing 4 takes us to Adrian orbit mode. Notice that the camera stays still when I do not touch the mouse. When I hold the left button and drag, I can look around Adrian from different sides and heights. The camera keeps the same distance and continues facing the planet. Dragging right moves the camera left around it, like moving the planet across the screen. Try a small vertical drag as well: changing elevation reveals more of the curved surface, while the fixed orbit distance keeps the planet framed consistently.

**Show:** Pause on a clear view of Adrian with stars behind it, then press Esc.

The tour gives us the dramatic approach, and the orbit view lets us explore the planet. Together, the two modes show both the planned presentation of the scene and the interactive controls behind it. That is the complete recreation. Thank you for watching.

## Recording checklist

- Keep the source text large enough to read while showing the linked lines.
- Use the code cues for Members 1–4 and the application output for Member 5.
- Rehearse once at a steady pace; use the pauses during the output segment to bring the presentation to about six minutes.
