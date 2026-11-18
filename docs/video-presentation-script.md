# Video Presentation Script: The Petrova Line Above Adrian

**Target duration:** about 5 minutes at a measured speaking pace.

**Presenters:** Members 1–4 explain the code. Member 5 demonstrates the running output.
**Recording rule:** Read only the text marked **Say**. The **Show** cues tell the screen operator exactly which code line or output to display; they are not spoken. Move to the next cue as the speaker begins its matching paragraph. Do not read the code line by line.

## 0:00–0:57 — Member 1: scene concept and program structure

**Show:** [README scene description](../README.md#L3).

**Say:** “Our project recreates a scene inspired by *Project Hail Mary*. Ryland Grace explores the Petrova Line above Adrian, where the glowing Astrophage field leads to a discovery that may help save two suns. We wanted the output to feel like a place the viewer can explore, so it has a ship, the planet, a crimson particle field, distant stars, and two camera views.”

**Show:** [Scene objects and camera variables](../main.cpp#L82), then [hardcoded camera endpoints](../main.cpp#L101).

**Say:** “These variables hold the two three-dimensional meshes, their textures, the current camera position and viewing angles, and the tour and orbit states. The Camera Tour starts and ends at the positions written here in the source, so it does not read camera files.”

**Show:** [`main()` and callback registration](../main.cpp#L765), then [`display()`](../main.cpp#L574).

**Say:** “`main()` prepares the window, meshes, textures, and controls. Every redraw goes through `display()`, which sets the camera, draws the background stars and three-dimensional objects, adds the particle field, and finally places the text overlay on top.”

## 0:57–1:50 — Member 2: camera movement and controls

**Show:** [`loadCameraPath()`](../main.cpp#L158), [`startAnimation()`](../main.cpp#L167), and [`applyPathPosition()`](../main.cpp#L137).

**Say:** “`loadCameraPath()` calculates the distance between the hardcoded endpoints. When key 3 calls `startAnimation()`, the tour begins again at the start. `applyPathPosition()` moves the camera between those endpoints and gradually changes its yaw and pitch. Those angle changes are the camera rotation used during the tour.”

**Show:** [`updateOrbitCamera()`](../main.cpp#L202), focusing on the fixed distance and aim calculations.

**Say:** “Key 4 enters the second view. `updateOrbitCamera()` places the camera on an imaginary sphere around Adrian and calculates the direction back toward the planet. This keeps Adrian in view and keeps the camera at the same distance while its angle or elevation changes.”

**Show:** [`keyboardDown()`](../main.cpp#L659), [`keyboardUp()`](../main.cpp#L720), [`mouseButton()`](../main.cpp#L688), [`mouseDrag()`](../main.cpp#L699), and [`timer()`](../main.cpp#L727).

**Say:** “The keyboard callbacks handle W, S, 3, 4, and Escape. `mouseButton()` detects when a left drag starts or ends, and `mouseDrag()` updates the orbit angle and elevation. The timer advances the tour according to elapsed time. Orbit mode stays still until the mouse is dragged.”

## 1:50–2:42 — Member 3: meshes, transformations, lighting, and textures

**Show:** [`initializeSceneMeshes()`](../main.cpp#L241), especially the planet vertex calculations.

**Say:** “The ship and Adrian are separate three-dimensional triangle meshes. `initializeSceneMeshes()` loads them, enlarges Adrian around its own center, and places it in the scene. That demonstrates scaling and translation. The ship also receives a translation when it is drawn.”

**Show:** [`uploadMesh()`](../main.cpp#L221), then [`drawMesh()`](../main.cpp#L486).

**Say:** “For the VBO requirement, `uploadMesh()` copies each mesh’s vertex data and triangle indices into GPU buffers. `drawMesh()` binds those buffers and draws the indexed triangles. The vertex data includes positions, surface normals, and texture coordinates, so the same mesh data supports shape, lighting, and texture placement.”

**Show:** [`loadTexture()`](../main.cpp#L265), [`drawMesh()` texture selection](../main.cpp#L488), and [ship and planet lighting](../main.cpp#L596).

**Say:** “`loadTexture()` prepares the image textures. `drawMesh()` maps one of Adrian’s textures onto the planet using its texture coordinates. In `display()`, one light gives the ship a colored bounce, while another lights Adrian. The ambient color also becomes redder as the tour reaches the Astrophage field.”

## 2:42–3:31 — Member 4: particles, stars, and the two-dimensional overlay

**Show:** [`initializeParticles()`](../main.cpp#L295) and [`animatedParticlePosition()`](../main.cpp#L344).

**Say:** “`initializeParticles()` gives the crimson particles different positions, sizes, colors, and appearance times. `animatedParticlePosition()` calculates where each particle belongs as the tour progresses. This produces the moving field that appears later in the scene.”

**Show:** [`drawParticleDisk()`](../main.cpp#L361) and [`drawParticles()`](../main.cpp#L380).

**Say:** “Each particle is drawn by `drawParticleDisk()` as a small camera-facing triangle fan with a soft, transparent edge. `drawParticles()` checks which particles are visible and blends them into a glow. Together, the ship, Adrian, and particle disks provide the three geometric objects in the scene.”

**Show:** [`data/stars_data.h` star count](../data/stars_data.h#L16), [`drawStars()`](../main.cpp#L464), and [`drawHud()`](../main.cpp#L520).

**Say:** “The separate star data file contains 250 background stars with varied positions, brightness, and sizes. `drawStars()` renders them behind the main objects. `drawHud()` changes to screen coordinates and draws the camera status, controls, and scene description as a two-dimensional text element over the three-dimensional view.”

## 3:31–5:00 — Member 5: live output demonstration

**Show:** Start the program at its 1920 by 1080 window size. Point to the ship, Adrian, background stars, and description. Do not show source code again.

**Say:** “Now I’ll show the finished output. The ship and Adrian are the main objects, while the scattered stars give the background depth. The text in the corners explains the scene and shows the current control hints. This is our interpretation of the Petrova Line above Adrian.”

**Show:** Hold W briefly, then S. Keep the camera tour in view.

**Say:** “Holding W moves the camera forward, and holding S moves it backward. These controls change our position along the tour without moving the planet itself.”

**Show:** Press 3 and let the tour progress until the glowing field appears.

**Say:** “Pressing 3 starts the automatic Camera Tour. As the view advances, the lighting shifts toward red, Adrian’s texture changes, and the glowing particles appear. The camera, lighting, textures, and particle animation work together to tell the story of the Astrophage discovery.”

**Show:** Press 4. Pause without dragging, then hold the left mouse button and drag horizontally and vertically.

**Say:** “Pressing 4 takes us to Adrian orbit mode. Notice that the camera stays still when I do not touch the mouse. When I hold the left button and drag, I can look around Adrian from different sides and heights. The camera keeps the same distance and continues facing the planet. Dragging right moves the camera left around it, like moving the planet across the screen.”

**Show:** Pause on a clear view of Adrian with stars behind it, then press Esc.

**Say:** “The tour gives us the dramatic approach, and the orbit view lets us explore the planet. That is the complete recreation. Thank you for watching.”

## Recording checklist

- Keep the source text large enough to read while showing the linked lines.
- Use the code cues for Members 1–4 and the application output for Member 5.
- Rehearse once at a steady pace; adjust pauses during the output segment to stay within five minutes.
