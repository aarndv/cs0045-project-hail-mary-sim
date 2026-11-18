/*
 * Embedded space scene (C++17, FreeGLUT, GLEW, fixed-function OpenGL).
 *
 * Requirements guide: search for the "REQUIREMENT:" comments below to find
 * the exact code that implements each requested feature.
 *
 * Controls:
 *   W / S (hold)  Move forward / backward along the camera path.
 *   3             Play or replay the camera tour.
 *   4             Orbit the camera around Adrian.
 *   Left drag     Change the orbit angle and elevation while orbit mode is active.
 *   Esc           Exit the application.
 * Camera path endpoints are defined directly in this source file.
 *
 * The two 3D meshes come from generated headers; stb_image loads the external
 * texture images. Together with those assets, this is the complete application
 * source. The generated mesh data is kept out of this file for readability.
 */
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#include "mesh_data.h"
#include "adrian_data.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#define BUFFER_OFFSET(i) ((void*)(i))

// --- Scene state --------------------------------------------------

// A position or direction in 3D space. Double precision keeps camera-path
// interpolation smooth before values are passed to OpenGL.
struct Vec3
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

// GPU handles and element count for one indexed 3D mesh.
struct RenderMesh
{
    GLuint vertexBuffer = 0;
    GLuint indexBuffer = 0;
    GLsizei indexCount = 0;
};

constexpr double shipMoveSpeed = 0.30;

// One saved camera-path endpoint, loaded from a CSV file.
struct CameraState
{
    Vec3 position;
    double yaw = 0.0;
    double pitch = 0.0;
};

// One red-field particle: its position, animation timing, color, and size.
struct Particle
{
    Vec3 position;
    float phase = 0.0f;
    float threshold = 0.0f;
    float tint = 1.0f;
    float crowding = 0.0f;
    float route = 0.0f;
    float haloScale = 1.0f;
};

// REQUIREMENT — geometric objects: the ship and Adrian planet are separate
// indexed 3D meshes. The particle field adds a third visible object: glowing
// camera-facing disks built from triangle fans in drawParticleDisk().
// Rendered assets and textures.
RenderMesh modelMesh, adrianMesh;
GLuint adrianTexture = 0, redAdrianTexture = 0;

// Camera path and interaction state. keys[] records held keys, while pathPosition
// is 0 at pos1 and 1 at pos2; animating enables automatic travel.
const Vec3 initialCamera{0.0, 0.0, 1.6 + adrian_data::modelZOffset};
Vec3 camera = initialCamera;
Vec3 particleAnchor = initialCamera;
double moveSpeed = shipMoveSpeed;
double cameraYaw = 0.0, cameraPitch = 0.0;
double particleYaw = 0.0, particlePitch = 0.0;
bool keys[256] = {};
int lastFrameTime = 0;
int windowWidth = 900, windowHeight = 700;
std::filesystem::path textureFileDirectory;
// Camera endpoints copied from the former pos1.csv and pos2.csv files.
CameraState animationStart{{-0.2169698016, 0.5950487339, -0.9267616077}, -1.5610, -0.0735};
CameraState animationEnd{{1.5565662165, 0.7131653820, -0.9155946978}, -1.5645, -0.0665};
bool pathReady = true;
bool animating = false;
bool orbitingAdrian = false;
bool draggingOrbit = false;
double orbitAngle = 0.0;
double orbitElevation = 0.08;
int previousMouseX = 0;
int previousMouseY = 0;
double pathPosition = 0.0, pathDistance = 0.0;

double randomUnit()
{
    return std::rand() / (RAND_MAX + 1.0);
}

// Generated once at startup and drawn as the path progresses.
Particle particles[30000] = {};

// --- Camera path --------------------------------------------------

// Convert camera yaw/pitch into the direction the camera looks.
// REQUIREMENT — rotation: yaw and pitch describe the view direction. display()
// passes that direction to gluLookAt, so the camera turns around the scene.
Vec3 forwardDirection()
{
    const double x = std::sin(cameraYaw) * std::cos(cameraPitch);
    const double y = std::sin(cameraPitch);
    const double z = -std::cos(cameraYaw) * std::cos(cameraPitch);
    return {x, y, z};
}

// Interpolate camera translation and yaw/pitch rotation along the path.
// The particle field follows only 10% of the camera's movement.
void applyPathPosition()
{
    constexpr double twoPi = 6.283185307179586;
    const double yawChange = std::remainder(animationEnd.yaw - animationStart.yaw, twoPi);
    camera.x = animationStart.position.x + (animationEnd.position.x - animationStart.position.x) * pathPosition;
    camera.y = animationStart.position.y + (animationEnd.position.y - animationStart.position.y) * pathPosition;
    camera.z = animationStart.position.z + (animationEnd.position.z - animationStart.position.z) * pathPosition;

    cameraYaw = animationStart.yaw + yawChange * pathPosition;
    cameraPitch = animationStart.pitch + (animationEnd.pitch - animationStart.pitch) * pathPosition;

    // The field drifts only one tenth as far as the camera travels.
    particleAnchor.x = animationStart.position.x + 0.10 * (camera.x - animationStart.position.x);
    particleAnchor.y = animationStart.position.y + 0.10 * (camera.y - animationStart.position.y);
    particleAnchor.z = animationStart.position.z + 0.10 * (camera.z - animationStart.position.z);

    particleYaw = animationStart.yaw + 0.10 * yawChange * pathPosition;
    particlePitch = animationStart.pitch + 0.10 * (cameraPitch - animationStart.pitch);
}

// Load both endpoints and compute the distance used for speed-independent travel.
void loadCameraPath()
{
    const Vec3 delta{animationEnd.position.x - animationStart.position.x, animationEnd.position.y - animationStart.position.y, animationEnd.position.z - animationStart.position.z};
    pathDistance = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    pathPosition = 0.0;
    applyPathPosition();
}

// Restart automatic playback from the beginning of the camera path.
void startAnimation()
{
    if (!pathReady) {
        glutPostRedisplay();
        return;
    }

    pathPosition = 0.0;
    applyPathPosition();
    animating = true;
    for (int i = 0; i < 256; ++i) keys[i] = false;
    glutPostRedisplay();
}

// --- Embedded meshes ----------------------------------------------

constexpr double adrianScale = 1.2;
constexpr double adrianDropFraction = 0.10;

// Place the enlarged planet relative to the ship and lower it slightly.
Vec3 adrianSceneCenter()
{
    const Vec3 original{adrian_data::centerX, adrian_data::centerY, adrian_data::centerZ};
    const Vec3 fromShip{original.x, original.y, original.z - adrian_data::modelZOffset};
    const double distance = std::sqrt(fromShip.x * fromShip.x + fromShip.y * fromShip.y + fromShip.z * fromShip.z);
    const double shift = (adrianScale - 1.0) * adrian_data::radius / distance;
    const Vec3 placed{original.x + fromShip.x * shift, original.y + fromShip.y * shift, original.z + fromShip.z * shift};
    const double centerDistance = distance + (adrianScale - 1.0) * adrian_data::radius;
    const double loweredY = placed.y - adrianDropFraction * (2.0 * adrian_data::radius * adrianScale);
    const double zFromShip = placed.z - adrian_data::modelZOffset;
    const double xFromShip = std::copysign(std::sqrt(std::max(0.0, centerDistance * centerDistance - loweredY * loweredY - zFromShip * zFromShip)), placed.x);
    return {xFromShip, loweredY, placed.z};
}

// Place the camera on a wide circular path around Adrian and aim it at the planet.
void updateOrbitCamera()
{
    const Vec3 target = adrianSceneCenter();
    const double orbitDistance = 600.0;

    const double horizontalDistance = orbitDistance * std::cos(orbitElevation);
    camera.x = target.x + std::cos(orbitAngle) * horizontalDistance;
    camera.y = target.y + std::sin(orbitElevation) * orbitDistance;
    camera.z = target.z + std::sin(orbitAngle) * horizontalDistance;

    const double directionX = target.x - camera.x;
    const double directionY = target.y - camera.y;
    const double directionZ = target.z - camera.z;
    cameraYaw = std::atan2(directionX, -directionZ);
    cameraPitch = std::atan2(directionY, std::sqrt(directionX * directionX + directionZ * directionZ));
}

// REQUIREMENT — VBO setup: each vertex stores position, normal, and UV data.
// This function uploads that vertex data and the triangle indices to GPU buffers.
void uploadMesh(const Mesh& mesh, RenderMesh& target)
{
    const std::size_t vertexBytes = mesh.vertices.size() * sizeof(Vertex);
    const std::size_t indexBytes = mesh.indices.size() * sizeof(GLuint);
    if (vertexBytes > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) || indexBytes > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) || mesh.indices.size() > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max())) {
        throw std::runtime_error("Mesh is too large for GPU buffers");
    }

    glGenBuffers(1, &target.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, target.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexBytes), mesh.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &target.indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, target.indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indexBytes), mesh.indices.data(), GL_STATIC_DRAW);
    target.indexCount = static_cast<GLsizei>(mesh.indices.size());
}

// Build the ship and planet GPU meshes. The planet's vertices are translated
// to their scene position and scaled about their original center on the CPU.
void initializeSceneMeshes()
{
    Mesh model = loadEmbeddedMesh();
    Mesh adrian = loadAdrianMesh();
    const Vec3 placed = adrianSceneCenter();
    // REQUIREMENT — scaling and translation: resize the planet around its
    // original center, then place its vertices at the calculated scene center.
    for (Vertex& vertex : adrian.vertices) {
        vertex.x = static_cast<GLfloat>(placed.x + (vertex.x - adrian_data::centerX) * adrianScale);
        vertex.y = static_cast<GLfloat>(placed.y + (vertex.y - adrian_data::centerY) * adrianScale);
        vertex.z = static_cast<GLfloat>(placed.z + (vertex.z - adrian_data::centerZ) * adrianScale);
    }

    std::cout << "Embedded scene: " << model.indices.size() / 3 << " model triangles and "
              << adrian.indices.size() / 3 << " Adrian triangles\n";

    uploadMesh(model, modelMesh);
    uploadMesh(adrian, adrianMesh);
}

// --- Textures -----------------------------------------------------

// REQUIREMENT — texture mapping: decode the image, configure repeat and linear
// filtering, then upload the pixels as an OpenGL 2D texture.
GLuint loadTexture(const std::filesystem::path& path)
{
    int width = 0, height = 0, channels = 0;
    unsigned char* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, 3);
    if (!pixels) throw std::runtime_error("Cannot load Adrian texture: " + path.string());
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);
    return texture;
}

// Load the normal and red planet textures; display() chooses by path progress.
void initializeSceneTextures()
{
    stbi_set_flip_vertically_on_load(1);
    adrianTexture = loadTexture(textureFileDirectory / "adrian_texture_map.jpg");
    redAdrianTexture = loadTexture(textureFileDirectory / "red_adrian_texture.png");
}

// --- Particle field -----------------------------------------------

// Give the particle field reproducible positions, timing, color, and sizes.
void initializeParticles()
{
    if (!pathReady) return;
    std::srand(2026);
    for (std::size_t i = 0; i < 30000; ++i) {
        Particle& particle = particles[i];
        const bool lateField = i >= 18000;
        const bool sideField = i >= 24000;

        double angle = randomUnit() * 6.283185307179586;
        if (sideField) {
        double side = 0.0;
            if (randomUnit() >= 0.5) side = 3.141592653589793;
            angle = (randomUnit() * 2.0 - 1.0) * 0.80 + side;
        }
        // Keep the outer volume circular while favoring its sides over the center.
        double fieldRadius = 2.45;
        if (i % 20 == 0) fieldRadius = 0.70;
        else if (i % 20 < 7) fieldRadius = 1.50;
        double radius = std::sqrt(randomUnit()) * fieldRadius;
        if (sideField) radius = 1.40 + randomUnit() * 0.80;
        double depth = 0.35 + randomUnit() * 3.30;
        if (i % 47 == 0) depth = 0.065 + randomUnit() * 0.065;
        else if (i % 31 == 0) depth = 0.08 + randomUnit() * 0.20;
        if (sideField) depth = 0.70 + randomUnit();
        particle.position = {std::cos(angle) * radius, std::sin(angle) * radius, depth};
        particle.phase = randomUnit() * 6.283185307179586;
        particle.threshold = randomUnit();
        if (lateField) particle.threshold = 0.50 + particle.threshold * 0.50;
        particle.tint = 0.65 + randomUnit() * 0.35;
        particle.route = randomUnit() * 0.90;
        if (lateField) particle.route = 0.58 + randomUnit() * 0.40;
        if (sideField) particle.route = 0.82 + randomUnit() * 0.13;
        if (randomUnit() < 0.08) particle.haloScale = 1.20 + randomUnit() * 1.30;
    }
    // Nearby projected particles share similar screen positions along the path.
    const auto cell = [](const Particle& p)
{
    const int x = std::clamp(static_cast<int>((p.position.x + 2.45) * 32.0 / 4.90), 0, 31);
        const int y = std::clamp(static_cast<int>((p.position.y + 2.45) * 32.0 / 4.90), 0, 31);
        return y * 32 + x;
    };
    int neighbors[32 * 32] = {};
    for (const Particle& particle : particles) ++neighbors[cell(particle)];
    for (Particle& particle : particles)
        particle.crowding = std::clamp((neighbors[cell(particle)] - 12) / 50.0f, 0.0f, 1.0f);
}

// Transform a particle from field coordinates into the animated 3D scene.
Vec3 animatedParticlePosition(const Particle& particle, const Vec3& forward, const Vec3& right, const Vec3& up)
{
    const double phase = particle.phase + pathPosition * 20.0;
    constexpr double tanHalfFov = 0.414213562373095;
    const double depth = particle.position.z;
    const double x = (particle.position.x + 0.018 * std::sin(phase)) * depth * tanHalfFov;
    const double y = (particle.position.y + 0.018 * std::cos(phase * 1.2)) * depth * tanHalfFov;

    Vec3 position;
    position.x = particleAnchor.x + (animationEnd.position.x - animationStart.position.x) * particle.route + forward.x * depth + right.x * x + up.x * y;
    position.y = particleAnchor.y + (animationEnd.position.y - animationStart.position.y) * particle.route + forward.y * depth + right.y * x + up.y * y;
    position.z = particleAnchor.z + (animationEnd.position.z - animationStart.position.z) * particle.route + forward.z * depth + right.z * x + up.z * y;
    return position;
}

// REQUIREMENT — geometric primitive: draw a glowing particle as a camera-facing
// triangle fan. The center and rim alpha values create its soft appearance.
void drawParticleDisk(const Vec3& position, const Vec3& right, const Vec3& up, double radius, const Vec3& color, float tint, float opacity, float rimOpacity = 0.0f)
{
    constexpr double twoPi = 6.283185307179586;
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(static_cast<GLfloat>(color.x * tint), static_cast<GLfloat>(color.y * tint), static_cast<GLfloat>(color.z * tint), opacity);
    glVertex3f(static_cast<GLfloat>(position.x), static_cast<GLfloat>(position.y), static_cast<GLfloat>(position.z));
    glColor4f(static_cast<GLfloat>(color.x * tint), static_cast<GLfloat>(color.y * tint), static_cast<GLfloat>(color.z * tint), rimOpacity);
    for (int corner = 0; corner <= 8; ++corner) {
        const double angle = twoPi * corner / 8.0;
        const double horizontal = radius * std::cos(angle);
        const double vertical = radius * std::sin(angle);
        glVertex3f(static_cast<GLfloat>(position.x + right.x * horizontal + up.x * vertical), static_cast<GLfloat>(position.y + right.y * horizontal + up.y * vertical), static_cast<GLfloat>(position.z + right.z * horizontal + up.z * vertical));
    }

    glEnd();
}

// Draw visible particles as additive, translucent disks after the opaque meshes.
// Lighting and depth writes are temporarily disabled for this glow effect.
void drawParticles()
{
    if (orbitingAdrian || !pathReady || pathPosition <= 0.40) return;

    const double density = std::min(1.0, (pathPosition - 0.40) / 0.60);
    const double endFade = std::clamp((pathPosition - 0.75) / 0.25, 0.0, 1.0);

    const Vec3 viewForward = forwardDirection();
    const Vec3 viewRight{std::cos(cameraYaw), 0.0, std::sin(cameraYaw)};
    const Vec3 viewUp{viewRight.y * viewForward.z - viewRight.z * viewForward.y, viewRight.z * viewForward.x - viewRight.x * viewForward.z, viewRight.x * viewForward.y - viewRight.y * viewForward.x};

    const Vec3 forward{std::sin(particleYaw) * std::cos(particlePitch), std::sin(particlePitch), -std::cos(particleYaw) * std::cos(particlePitch)};
    const Vec3 right{std::cos(particleYaw), 0.0, std::sin(particleYaw)};
    const Vec3 up{right.y * forward.z - right.z * forward.y, right.z * forward.x - right.x * forward.z, right.x * forward.y - right.y * forward.x};

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    for (int i = 0; i < static_cast<int>(30000); ++i) {
        const Particle& particle = particles[i];
        if (particle.threshold > density) continue;

        const Vec3 position = animatedParticlePosition(particle, forward, right, up);
        const Vec3 offset{position.x - camera.x, position.y - camera.y, position.z - camera.z};
        const double depth = offset.x * viewForward.x + offset.y * viewForward.y + offset.z * viewForward.z;
        if (depth <= 0.055) continue;

        const double farFade = std::clamp((depth - 1.2) / 2.5, 0.0, 1.0);
        const double visibility = 1.0 - 0.90 * endFade * farFade;
        if (particle.threshold > density * visibility) continue;

        const double screenX = (offset.x * viewRight.x + offset.y * viewRight.y + offset.z * viewRight.z) / (depth * 0.414213562373095);
        const double screenY = (offset.x * viewUp.x + offset.y * viewUp.y + offset.z * viewUp.z) / (depth * 0.414213562373095);
        if (std::abs(screenX) > static_cast<double>(windowWidth) / windowHeight + 0.25 || std::abs(screenY) > 1.25) continue;
        if (screenX * screenX + screenY * screenY < 0.65 * 0.65 && i % 13 < 5) continue;

        const double pixelScale = 0.828427124746 * depth / windowHeight;
        const int sizeClass = i % 20;

        double basePixels = 4.5;
        if (sizeClass >= 14) basePixels = 8.0;
        if (sizeClass >= 19) basePixels = 14.0;

        double depthMultiplier = 2.0;
        if (particle.position.z < 0.30) depthMultiplier = 3.0;
        if (particle.position.z < 0.15) depthMultiplier = 4.0;

        double distanceMultiplier = 0.7 + 0.45 / depth;
        if (depthMultiplier < distanceMultiplier) distanceMultiplier = depthMultiplier;
        const double pixels = basePixels * distanceMultiplier;

        Vec3 color{1.0, 0.70, 0.82};
        if (sizeClass < 19) color = {1.0, 0.28, 0.52};
        if (sizeClass < 14) color = {0.75, 0.08, 0.28};
        const float tint = particle.tint * (1.0f + 0.25f * particle.crowding);

        drawParticleDisk(position, viewRight, viewUp, pixels * pixelScale * particle.haloScale, color, tint, static_cast<float>((0.50f + 0.35f * particle.crowding) * visibility));
        if (depth < 0.35 && particle.position.z < 0.30 && particle.phase < 2.1f)
            drawParticleDisk(position, viewRight, viewUp, pixels * pixelScale * 0.22, {1.0, 0.82, 0.90}, 1.0f, 0.95f, 0.95f);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// --- Rendering and HUD --------------------------------------------

// Keep the viewport and 3D perspective projection matched to the window size.
void reshape(int width, int height)
{
    width = std::max(width, 1);
    height = std::max(height, 1);
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, static_cast<double>(width) / height, 0.05, 2000.0);
}

// REQUIREMENT — VBO and 3D mesh: bind the mesh buffers, describe the interleaved
// vertex attributes, and render its indexed triangles.
void drawMesh(const RenderMesh& mesh, bool textured)
{
    // REQUIREMENT — texture mapping: bind Adrian's normal or red texture and
    // enable the mesh UV coordinates for the planet draw.
    if (textured) {
        glEnable(GL_TEXTURE_2D);
        GLuint texture = adrianTexture;
        if (pathPosition >= 0.40) texture = redAdrianTexture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(0));
    glNormalPointer(GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(3 * sizeof(GLfloat)));
    if (textured) glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(6 * sizeof(GLfloat)));
    // REQUIREMENT — 3D elements: draw the ship and planet from their indexed
    // triangle meshes in the perspective camera view.
    glDrawElements(GL_TRIANGLES, mesh.indexCount, static_cast<GLenum>(0x1405), BUFFER_OFFSET(0));
    if (textured) {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_TEXTURE_2D);
    }
}

void hudText(float x, float y, const std::string& line, void* font)
{
    glRasterPos2f(x, y);
    for (unsigned char letter : line) glutBitmapCharacter(font, letter);
}

// REQUIREMENT — 2D element: switch to screen coordinates and draw readable HUD
// text over the perspective-rendered 3D scene.
void drawHud()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    const float left = 24.0f;
    const float top = static_cast<float>(windowHeight - 28);
    std::string status = "Paused on camera path";
    if (orbitingAdrian) status = "Orbiting Adrian";
    else if (animating) status = "Playing camera path";
    else if (pathPosition <= 0.0) status = "At the starting position";
    else if (pathPosition >= 1.0) status = "At the destination";
    else if (keys['w'] && !keys['s']) status = "Moving forward";
    else if (keys['s'] && !keys['w']) status = "Moving backward";

    glColor3f(0.94f, 0.97f, 0.98f);
    hudText(left, top, "CAMERA PATH", GLUT_BITMAP_HELVETICA_18);
    hudText(left, top - 26, status, GLUT_BITMAP_9_BY_15);

    glColor3f(0.68f, 0.76f, 0.79f);
    hudText(left, top - 70, "Hold W/S: move  |  3: Camera Tour  |  4: Orbit Adrian  |  Esc: exit", GLUT_BITMAP_8_BY_13);
    if (orbitingAdrian) hudText(left, top - 100, "DRAG USING YOUR MOUSE TO MOVE", GLUT_BITMAP_HELVETICA_18);

    const float descriptionX = static_cast<float>(std::max(360, windowWidth - 370));
    glColor3f(0.94f, 0.97f, 0.98f);
    hudText(descriptionX, top, "The Petrova Line Above Adrian", GLUT_BITMAP_9_BY_15);

    glColor3f(0.78f, 0.84f, 0.87f);
    hudText(descriptionX, top - 24, "Orbiting the green exoplanet Adrian,", GLUT_BITMAP_8_BY_13);
    hudText(descriptionX, top - 40, "astronaut Ryland Grace samples a glowing", GLUT_BITMAP_8_BY_13);
    hudText(descriptionX, top - 56, "crimson arc of solar-draining Astrophage.", GLUT_BITMAP_8_BY_13);
    hudText(descriptionX, top - 72, "The breakthrough reveals a hidden microbial", GLUT_BITMAP_8_BY_13);
    hudText(descriptionX, top - 88, "ecosystem capable of saving both of their", GLUT_BITMAP_8_BY_13);
    hudText(descriptionX, top - 104, "dying suns.", GLUT_BITMAP_8_BY_13);
    hudText(descriptionX, top - 132, "Feel free to explore this recreation of the", GLUT_BITMAP_8_BY_13);
    hudText(descriptionX, top - 148, "scene from the movie Project Hail Mary.", GLUT_BITMAP_8_BY_13);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Render one frame: update lighting, aim the camera, draw the 3D objects,
// composite the particle field and 2D HUD, then swap the back buffer.
void display()
{
    const float redMix = static_cast<float>(std::clamp((pathPosition - 0.40) / 0.07, 0.0, 1.0));
    const float spawn = static_cast<float>(std::clamp((pathPosition - 0.40) / 0.60, 0.0, 1.0));
    const float particleAmount = 0.60f * spawn + 0.40f * std::max(0.0f, 2.0f * spawn - 1.0f);

    glClearColor(0.003f + 0.087f * redMix, 0.005f + 0.007f * redMix, 0.012f + 0.020f * redMix, 1.0f);
    const GLfloat spaceAmbient[] = {0.003f + 0.15f * redMix, 0.004f + 0.012f * redMix, 0.005f + 0.025f * redMix, 1.0f};
    // REQUIREMENT — lighting: raise the scene's ambient red contribution as
    // the Astrophage field appears along the camera tour.
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, spaceAmbient);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    const Vec3 forward = forwardDirection();

    // REQUIREMENT — translation and rotation: position the camera at camera
    // and aim it using the direction calculated from yaw and pitch.
    gluLookAt(camera.x, camera.y, camera.z, camera.x + forward.x, camera.y + forward.y, camera.z + forward.z, 0.0, 1.0, 0.0);

    // REQUIREMENT — lighting: GL_LIGHT2 gives the ship a colored bounce light.
    // After drawing the ship, GL_LIGHT0 illuminates Adrian like a distant star.
    glDisable(GL_LIGHT0);
    const GLfloat topReflectionDirection[] = {-0.35f, 1.0f, 0.12f, 0.0f};
    const GLfloat topReflectionDiffuse[] = {0.04f + 0.26f * redMix + 0.32f * particleAmount, 0.24f - 0.15f * redMix, 0.056f + 0.064f * redMix, 1.0f};
    const GLfloat topReflectionSpecular[] = {0.024f + 0.18f * redMix + 0.22f * particleAmount, 0.288f - 0.19f * redMix, 0.048f + 0.05f * redMix, 1.0f};
    glLightfv(GL_LIGHT2, GL_POSITION, topReflectionDirection);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, topReflectionDiffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, topReflectionSpecular);
    glEnable(GL_LIGHT2);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glPushMatrix();
    // REQUIREMENT — translation: move the ship mesh along Z before drawing it.
    glTranslatef(0.0f, 0.0f, adrian_data::modelZOffset);
    const GLfloat shipAmbient[] = {0.40f, 0.15f, 0.18f, 1.0f};
    const GLfloat shipDiffuse[] = {0.58f, 0.62f, 0.66f, 1.0f};
    const GLfloat shipSpecular[] = {0.55f, 0.55f, 0.55f, 1.0f};
    const GLfloat shipShininess[] = {44.0f};
    const GLfloat shipEmission[] = {0.18f * particleAmount, 0.006f * particleAmount, 0.030f * particleAmount, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, shipAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, shipDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shipSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shipShininess);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, shipEmission);

    drawMesh(modelMesh, false);
    glPopMatrix();
    const GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);
    glDisable(GL_LIGHT2);
    glEnable(GL_LIGHT0);
    const GLfloat planetSpecular[] = {0.0f, 0.0f, 0.0f, 1.0f};
    const GLfloat planetDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    const GLfloat planetAmbient[] = {0.18f, 0.20f, 0.14f, 1.0f};
    const GLfloat planetStarDiffuse[] = {0.62f + 0.38f * redMix, 1.0f - 0.70f * redMix, 0.24f - 0.02f * redMix, 1.0f};
    const GLfloat planetStarDirection[] = {0.2f, 1.0f, 0.1f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, planetStarDirection);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, planetStarDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, planetAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, planetDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, planetSpecular);

    drawMesh(adrianMesh, true);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    drawParticles();
    drawHud();
    glutSwapBuffers();
}

// --- Keyboard and timer -------------------------------------------

// Treat uppercase W/S the same as lowercase keys.
unsigned char movementKey(unsigned char key)
{
    if (key >= 'A' && key <= 'Z') return key - 'A' + 'a';
    return key;
}

// REQUIREMENT — keyboard callback: Esc exits, 3 starts the camera tour, 4 enters
// Adrian orbit mode, and holding W or S moves along the camera tour.
void keyboardDown(unsigned char key, int, int)
{
    key = movementKey(key);
    if (key == 27) std::exit(0);
    if (key == '3') {
        orbitingAdrian = false;
        startAnimation();
        return;
    }

    if (key == '4') {
        orbitingAdrian = true;
        animating = false;
        orbitAngle = 0.0;
        orbitElevation = 0.08;
        updateOrbitCamera();
        glutPostRedisplay();
        return;
    }

    if (pathReady && (key == 'w' || key == 's')) {
        animating = false;
        keys[key] = true;
        glutPostRedisplay();
    }
}

// REQUIREMENT — mouse callback: track left-button press/release while orbit
// mode is active so mouseDrag can update the camera only during a drag.
void mouseButton(int button, int state, int x, int y)
{
    if (!orbitingAdrian || button != GLUT_LEFT_BUTTON) return;

    draggingOrbit = (state == GLUT_DOWN);
    previousMouseX = x;
    previousMouseY = y;
}

// REQUIREMENT — mouse interaction: horizontal and vertical drag change the
// orbit angle and elevation; updateOrbitCamera keeps the view aimed at Adrian.
void mouseDrag(int x, int y)
{
    if (!orbitingAdrian || !draggingOrbit) return;

    const int deltaX = x - previousMouseX;
    const int deltaY = y - previousMouseY;
    previousMouseX = x;
    previousMouseY = y;

    // Dragging the scene right moves the camera left around the planet.
    orbitAngle += deltaX * 0.004;
    orbitElevation += deltaY * 0.004;

    if (orbitElevation > 1.2) orbitElevation = 1.2;
    if (orbitElevation < -1.2) orbitElevation = -1.2;

    updateOrbitCamera();
    glutPostRedisplay();
}

// Stop manual movement when W or S is released.
void keyboardUp(unsigned char key, int, int)
{
    keys[movementKey(key)] = false;
    glutPostRedisplay();
}

// Advance the path every ~16 ms using elapsed time, then request a redraw.
void timer(int)
{
    const int now = glutGet(GLUT_ELAPSED_TIME);
    const double seconds = std::min((now - lastFrameTime) / 1000.0, 0.05);
    lastFrameTime = now;

    int direction = 0;
    if (animating) direction = 1;
    else {
        if (keys['w']) ++direction;
        if (keys['s']) --direction;
    }

    if (!orbitingAdrian && pathReady && direction != 0) {
        const double previous = pathPosition;
        if (pathDistance > 0.0) {
            pathPosition += direction * moveSpeed * seconds / pathDistance;
            if (pathPosition < 0.0) pathPosition = 0.0;
            if (pathPosition > 1.0) pathPosition = 1.0;
        } else {
            pathPosition = 1.0;
        }

        if (pathPosition != previous) {
            applyPathPosition();
            glutPostRedisplay();
        }

        if (pathPosition == 0.0 || pathPosition == 1.0) {
            animating = false;
            glutPostRedisplay();
        }
    }

    glutTimerFunc(16, timer, 0);
}

// Initialize the window/OpenGL state and register all rendering/input callbacks.
int main(int argc, char** argv)
{
    try {
        const auto projectRoot = std::filesystem::absolute(argv[0]).parent_path();
        textureFileDirectory = projectRoot / "assets" / "textures";
        loadCameraPath();
        initializeParticles();
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
        glutInitWindowSize(1920, 1080);
        glutInitWindowPosition(100, 100);
        glutCreateWindow("The Petrova Line Above Adrian");
        if (glewInit() != GLEW_OK) throw std::runtime_error("GLEW initialization failed");

        reshape(1920, 1080);
        glClearColor(0.003f, 0.005f, 0.012f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        const GLfloat spaceAmbient[] = {0.003f, 0.004f, 0.005f, 1.0f};
        const GLfloat starAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
        const GLfloat noAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, spaceAmbient);
        glLightfv(GL_LIGHT0, GL_AMBIENT, starAmbient);
        glLightfv(GL_LIGHT2, GL_AMBIENT, noAmbient);

        initializeSceneMeshes();
        initializeSceneTextures();

        std::cout << "Controls: hold W/S to move forward or backward; "
                     "3 for Camera Tour; 4 to orbit Adrian; Esc to exit\n";
        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glutKeyboardFunc(keyboardDown);
        glutKeyboardUpFunc(keyboardUp);
        glutMouseFunc(mouseButton);
        glutMotionFunc(mouseDrag);
        glutIgnoreKeyRepeat(1);
        lastFrameTime = glutGet(GLUT_ELAPSED_TIME);
        glutTimerFunc(16, timer, 0);
        glutMainLoop();
    } catch (const std::exception& e)
{
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
