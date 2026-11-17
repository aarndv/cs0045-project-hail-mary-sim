#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#include "mesh_data.h"
#include "adrian_data.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#define BUFFER_OFFSET(i) ((void*)(i))

namespace {

struct Vec3 {
    double x = 0.0, y = 0.0, z = 0.0;
};

struct Vec2 {
    double u = 0.0, v = 0.0;
};

static_assert(sizeof(Vertex) == 8 * sizeof(GLfloat), "Vertex must be tightly interleaved");

struct RenderMesh {
    GLuint vertexBuffer = 0;
    GLuint indexBuffer = 0;
    GLsizei indexCount = 0;
};
RenderMesh modelMesh, adrianMesh;
GLuint adrianTexture = 0, redAdrianTexture = 0;
const Vec3 initialCamera{0.0, 0.0, 1.6 + adrian_data::modelZOffset};
constexpr double adrianScale = 1.2; // 60% of Adrian's previous diameter.
constexpr double adrianDropFraction = 0.10; // Fraction of its displayed diameter.
constexpr double shipMoveSpeed = 0.30;
Vec3 camera = initialCamera;
double moveSpeed = shipMoveSpeed;
double cameraYaw = 0.0;
double cameraPitch = 0.0;
std::array<bool, 256> keys{};
int lastFrameTime = 0;
int windowWidth = 900, windowHeight = 700;
std::filesystem::path cameraFileDirectory;
std::string hudStatus = "Loading pos1.csv and pos2.csv...";

struct CameraState {
    Vec3 position;
    double yaw = 0.0, pitch = 0.0, speed = shipMoveSpeed;
};
CameraState animationStart, animationEnd;
bool pathReady = false;
bool animating = false;
double pathPosition = 0.0, pathDistance = 0.0;

struct Particle {
    Vec3 position;
    float phase = 0.0f, threshold = 0.0f, tint = 1.0f;
    bool followsCamera = false;
};
std::array<Particle, 6000> particles{};

Vec3 adrianSceneCenter() {
    const Vec3 original{adrian_data::centerX, adrian_data::centerY, adrian_data::centerZ};
    const Vec3 fromShip{original.x, original.y, original.z - adrian_data::modelZOffset};
    const double distance = std::sqrt(fromShip.x * fromShip.x + fromShip.y * fromShip.y +
                                      fromShip.z * fromShip.z);
    const double shift = (adrianScale - 1.0) * adrian_data::radius / distance;
    const Vec3 placed{original.x + fromShip.x * shift,
                      original.y + fromShip.y * shift,
                      original.z + fromShip.z * shift};
    const double centerDistance = distance + (adrianScale - 1.0) * adrian_data::radius;
    const double loweredY = placed.y - adrianDropFraction *
        (2.0 * adrian_data::radius * adrianScale);
    const double zFromShip = placed.z - adrian_data::modelZOffset;
    const double xFromShip = std::copysign(std::sqrt(std::max(0.0,
        centerDistance * centerDistance - loweredY * loweredY - zFromShip * zFromShip)), placed.x);
    return {xFromShip, loweredY, placed.z};
}

Vec3 forwardDirection() {
    return {std::sin(cameraYaw) * std::cos(cameraPitch),
            std::sin(cameraPitch),
            -std::cos(cameraYaw) * std::cos(cameraPitch)};
}

bool readCamera(const char* name, CameraState& state) {
    std::ifstream in(cameraFileDirectory / name);
    std::string header, row, field;
    if (!std::getline(in, header) || header !=
        "x,y,z,look_x,look_y,look_z,yaw,pitch,speed,fov" || !std::getline(in, row)) return false;
    std::istringstream values(row);
    std::array<double, 10> numbers{};
    try {
        for (double& number : numbers) {
            if (!std::getline(values, field, ',')) return false;
            std::size_t used = 0;
            number = std::stod(field, &used);
            if (used != field.size() || !std::isfinite(number)) return false;
        }
    } catch (const std::exception&) {
        return false;
    }
    if (std::getline(values, field, ',') || numbers[8] <= 0.0 || numbers[9] != 45.0) return false;
    state = {{numbers[0], numbers[1], numbers[2]}, numbers[6], numbers[7], numbers[8]};
    return true;
}

void applyPathPosition() {
    constexpr double twoPi = 6.283185307179586;
    const double yawChange = std::remainder(animationEnd.yaw - animationStart.yaw, twoPi);
    camera = {animationStart.position.x +
                  (animationEnd.position.x - animationStart.position.x) * pathPosition,
              animationStart.position.y +
                  (animationEnd.position.y - animationStart.position.y) * pathPosition,
              animationStart.position.z +
                  (animationEnd.position.z - animationStart.position.z) * pathPosition};
    cameraYaw = animationStart.yaw + yawChange * pathPosition;
    cameraPitch = animationStart.pitch +
                  (animationEnd.pitch - animationStart.pitch) * pathPosition;
}

void loadCameraPath() {
    pathReady = readCamera("pos1.csv", animationStart) &&
                readCamera("pos2.csv", animationEnd);
    if (!pathReady) {
        hudStatus = "Missing or invalid pos1.csv / pos2.csv.";
        return;
    }
    const Vec3 delta{animationEnd.position.x - animationStart.position.x,
                     animationEnd.position.y - animationStart.position.y,
                     animationEnd.position.z - animationStart.position.z};
    pathDistance = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    pathPosition = 0.0;
    applyPathPosition();
    hudStatus = "At pos1. Hold W to move toward pos2.";
}

void initializeParticles() {
    if (!pathReady) return;
    std::mt19937 random(2026);
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    const double yaw = animationStart.yaw, pitch = animationStart.pitch;
    const Vec3 forward{std::sin(yaw) * std::cos(pitch), std::sin(pitch),
                       -std::cos(yaw) * std::cos(pitch)};
    const Vec3 right{std::cos(yaw), 0.0, std::sin(yaw)};
    const Vec3 up{right.y * forward.z - right.z * forward.y,
                  right.z * forward.x - right.x * forward.z,
                  right.x * forward.y - right.y * forward.x};
    for (std::size_t i = 0; i < particles.size(); ++i) {
        Particle& particle = particles[i];
        particle.followsCamera = i % 4 == 0;
        if (particle.followsCamera) {
            // A circular screen-space layer keeps the sides populated at pos2.
            const bool fillsCenter = i % 12 == 0;
            const double angle = fillsCenter ? unit(random) * 6.283185307179586 :
                (unit(random) * 2.0 - 1.0) * 1.1;
            const double side = fillsCenter || unit(random) < 0.5 ? -1.0 : 1.0;
            const double radius = fillsCenter ? std::sqrt(unit(random)) * 0.98 :
                0.66 + unit(random) * 0.32;
            particle.position = {side * std::cos(angle) * radius,
                                 std::sin(angle) * radius,
                                 0.4 + unit(random) * 1.4};
        } else {
            const double route = unit(random);
            const double depth = 0.45 + unit(random) * 3.5;
            const double lateral = unit(random) * 2.0 - 1.0;
            // Spread the field across the frame and favor its left and right edges.
            const double side = std::copysign(std::sqrt(std::abs(lateral)), lateral) * depth * 0.52;
            const double height = (unit(random) - 0.5) * depth * 0.85;
            const Vec3 anchor{
                animationStart.position.x + (animationEnd.position.x - animationStart.position.x) * route,
                animationStart.position.y + (animationEnd.position.y - animationStart.position.y) * route,
                animationStart.position.z + (animationEnd.position.z - animationStart.position.z) * route};
            particle.position = {anchor.x + forward.x * depth + right.x * side + up.x * height,
                                 anchor.y + forward.y * depth + right.y * side + up.y * height,
                                 anchor.z + forward.z * depth + right.z * side + up.z * height};
        }
        particle.phase = static_cast<float>(unit(random) * 6.283185307179586);
        particle.threshold = static_cast<float>(unit(random));
        particle.tint = static_cast<float>(0.65 + unit(random) * 0.35);
    }
}

void startAnimation() {
    if (!pathReady) {
        hudStatus = "Missing or invalid pos1.csv / pos2.csv.";
        glutPostRedisplay();
        return;
    }
    pathPosition = 0.0;
    applyPathPosition();
    animating = true;
    keys.fill(false);
    hudStatus = "Playing pos1 -> pos2. W/S takes manual control.";
    glutPostRedisplay();
}

void reshape(int width, int height) {
    width = std::max(width, 1);
    height = std::max(height, 1);
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, static_cast<double>(width) / height, 0.05, 2000.0);
}

void drawMesh(const RenderMesh& mesh, bool textured) {
    if (textured) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, pathPosition >= 0.40 ? redAdrianTexture : adrianTexture);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(0));
    glNormalPointer(GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(3 * sizeof(GLfloat)));
    if (textured) glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(6 * sizeof(GLfloat)));
    glDrawElements(GL_TRIANGLES, mesh.indexCount, static_cast<GLenum>(0x1405), BUFFER_OFFSET(0));
    if (textured) {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_TEXTURE_2D);
    }
}

void hudText(float x, float y, const std::string& line) {
    glRasterPos2f(x, y);
    for (unsigned char letter : line) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, letter);
}

void drawHud() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    const float top = static_cast<float>(windowHeight);
    glColor3f(0.08f, 0.16f, 0.14f);
    glBegin(GL_QUADS);
    glVertex2f(10.0f, top - 34.0f);
    glVertex2f(145.0f, top - 34.0f);
    glVertex2f(145.0f, top - 8.0f);
    glVertex2f(10.0f, top - 8.0f);
    glEnd();
    glColor3f(0.88f, 1.0f, 0.92f);
    hudText(18.0f, top - 25.0f, "Play [3]");
    glColor3f(0.78f, 0.88f, 0.83f);
    hudText(10.0f, top - 52.0f,
            "W/S: scrub camera | red field and Adrian at 40% | 3: autoplay | Esc: exit");
    const std::string progress = pathReady ?
        "  Path: " + std::to_string(static_cast<int>(pathPosition * 100.0 + 0.5)) + "%" : "";
    hudText(10.0f, top - 70.0f, hudStatus + progress);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

Vec3 animatedParticlePosition(const Particle& particle) {
    const double phase = particle.phase + pathPosition * 20.0;
    if (particle.followsCamera) {
        const Vec3 forward = forwardDirection();
        const Vec3 right{std::cos(cameraYaw), 0.0, std::sin(cameraYaw)};
        const Vec3 up{right.y * forward.z - right.z * forward.y,
                      right.z * forward.x - right.x * forward.z,
                      right.x * forward.y - right.y * forward.x};
        constexpr double tanHalfFov = 0.414213562373095;
        const double depth = particle.position.z;
        const double x = (particle.position.x + 0.018 * std::sin(phase)) *
                         depth * tanHalfFov;
        const double y = (particle.position.y + 0.018 * std::cos(phase * 1.2)) *
                         depth * tanHalfFov;
        return {camera.x + forward.x * depth + right.x * x + up.x * y,
                camera.y + forward.y * depth + right.y * x + up.y * y,
                camera.z + forward.z * depth + right.z * x + up.z * y};
    }
    return {particle.position.x + 0.12 * std::sin(phase),
            particle.position.y + 0.10 * std::cos(phase * 1.2),
            particle.position.z + 0.08 * std::sin(phase * 0.8)};
}

void drawGlowDisk(const Vec3& position, const Vec3& right, const Vec3& up,
                  double radius, const Vec3& color, float tint, float opacity) {
    constexpr double twoPi = 6.283185307179586;
    // Additive blending fades the rim into the scene without a dark outline.
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(static_cast<GLfloat>(color.x * tint),
              static_cast<GLfloat>(color.y * tint),
              static_cast<GLfloat>(color.z * tint), opacity);
    glVertex3f(static_cast<GLfloat>(position.x), static_cast<GLfloat>(position.y),
               static_cast<GLfloat>(position.z));
    glColor4f(static_cast<GLfloat>(color.x * tint),
              static_cast<GLfloat>(color.y * tint),
              static_cast<GLfloat>(color.z * tint), 0.0f);
    for (int corner = 0; corner <= 12; ++corner) {
        const double angle = twoPi * corner / 12.0;
        const double horizontal = radius * std::cos(angle);
        const double vertical = radius * std::sin(angle);
        glVertex3f(static_cast<GLfloat>(position.x + right.x * horizontal + up.x * vertical),
                   static_cast<GLfloat>(position.y + right.y * horizontal + up.y * vertical),
                   static_cast<GLfloat>(position.z + right.z * horizontal + up.z * vertical));
    }
    glEnd();
}

void drawParticles() {
    if (!pathReady || pathPosition <= 0.40) return;
    const double density = std::min(1.0, (pathPosition - 0.40) / 0.60);
    const Vec3 forward = forwardDirection();
    const Vec3 right{std::cos(cameraYaw), 0.0, std::sin(cameraYaw)};
    const Vec3 up{right.y * forward.z - right.z * forward.y,
                  right.z * forward.x - right.x * forward.z,
                  right.x * forward.y - right.y * forward.x};
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for (int i = 0; i < static_cast<int>(particles.size()); ++i) {
        const Particle& particle = particles[i];
        if (particle.threshold > density) continue;
        const Vec3 position = animatedParticlePosition(particle);
        const Vec3 offset{position.x - camera.x, position.y - camera.y,
                          position.z - camera.z};
        const double depth = offset.x * forward.x + offset.y * forward.y + offset.z * forward.z;
        if (depth <= 0.06) continue;
        const double pixelScale = 0.828427124746 * depth / windowHeight;
        const Vec3 color = i < 4200 ? Vec3{0.70, 0.08, 0.25} :
                           (i < 5720 ? Vec3{1.0, 0.22, 0.46} : Vec3{1.0, 0.58, 0.72});
        if (i < 4200) {
            drawGlowDisk(position, right, up, 5.0 * pixelScale, color, particle.tint, 0.55f);
        } else {
            const double haloPixels = i < 5720 ? 12.0 : 20.0;
            const double corePixels = i < 5720 ? 3.5 : 5.0;
            drawGlowDisk(position, right, up, haloPixels * pixelScale, color,
                         particle.tint, 0.32f);
            drawGlowDisk(position, right, up, corePixels * pixelScale, color,
                         particle.tint, 0.86f);
        }
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void display() {
    const float redMix = static_cast<float>(std::clamp((pathPosition - 0.40) / 0.07, 0.0, 1.0));
    glClearColor(0.003f + 0.087f * redMix, 0.005f + 0.007f * redMix,
                 0.012f + 0.020f * redMix, 1.0f);
    const GLfloat spaceAmbient[] = {0.003f + 0.15f * redMix,
                                    0.004f + 0.012f * redMix,
                                    0.005f + 0.025f * redMix, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, spaceAmbient);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    const Vec3 forward = forwardDirection();
    gluLookAt(camera.x, camera.y, camera.z,
              camera.x + forward.x, camera.y + forward.y, camera.z + forward.z,
              0.0, 1.0, 0.0);

    // Only the green bounce lights the ship; Adrian's star lighting is set afterward.
    glDisable(GL_LIGHT0);
    const GLfloat topReflectionDirection[] = {-0.35f, 1.0f, 0.12f, 0.0f};
    const GLfloat topReflectionDiffuse[] = {0.04f + 0.26f * redMix,
                                            0.24f - 0.15f * redMix,
                                            0.056f + 0.064f * redMix, 1.0f};
    const GLfloat topReflectionSpecular[] = {0.024f + 0.18f * redMix,
                                             0.288f - 0.19f * redMix,
                                             0.048f + 0.05f * redMix, 1.0f};
    glLightfv(GL_LIGHT2, GL_POSITION, topReflectionDirection);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, topReflectionDiffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, topReflectionSpecular);
    glEnable(GL_LIGHT2);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, adrian_data::modelZOffset);
    const GLfloat shipAmbient[] = {0.40f, 0.15f, 0.18f, 1.0f};
    const GLfloat shipDiffuse[] = {0.58f, 0.62f, 0.66f, 1.0f};
    const GLfloat shipSpecular[] = {0.55f, 0.55f, 0.55f, 1.0f};
    const GLfloat shipShininess[] = {44.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, shipAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, shipDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shipSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shipShininess);
    drawMesh(modelMesh, false);
    glPopMatrix();
    glDisable(GL_LIGHT2);
    glEnable(GL_LIGHT0);
    const GLfloat planetSpecular[] = {0.0f, 0.0f, 0.0f, 1.0f};
    const GLfloat planetDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    const GLfloat planetAmbient[] = {0.18f, 0.20f, 0.14f, 1.0f};
    const GLfloat planetStarDiffuse[] = {0.62f + 0.38f * redMix,
                                         1.0f - 0.70f * redMix,
                                         0.24f - 0.02f * redMix, 1.0f};
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

unsigned char movementKey(unsigned char key) {
    return key >= 'A' && key <= 'Z' ? key - 'A' + 'a' : key;
}

void keyboardDown(unsigned char key, int, int) {
    key = movementKey(key);
    if (key == 27) std::exit(0);
    if (key == '3') {
        startAnimation();
        return;
    }
    if (pathReady && (key == 'w' || key == 's')) {
        animating = false;
        keys[key] = true;
        hudStatus = key == 'w' ? "Moving toward pos2." : "Moving toward pos1.";
        glutPostRedisplay();
    }
}

void keyboardUp(unsigned char key, int, int) {
    keys[movementKey(key)] = false;
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN &&
        x >= 10 && x < 145 && y >= 8 && y < 34) startAnimation();
}

void timer(int) {
    const int now = glutGet(GLUT_ELAPSED_TIME);
    const double seconds = std::min((now - lastFrameTime) / 1000.0, 0.05);
    lastFrameTime = now;

    const int direction = animating ? 1 :
        static_cast<int>(keys['w']) - static_cast<int>(keys['s']);
    if (pathReady && direction != 0) {
        const double previous = pathPosition;
        pathPosition = pathDistance > 0.0 ?
            std::max(0.0, std::min(1.0,
                pathPosition + direction * moveSpeed * seconds / pathDistance)) : 1.0;
        if (pathPosition != previous) {
            applyPathPosition();
            glutPostRedisplay();
        }
        if (pathPosition == 0.0 || pathPosition == 1.0) {
            animating = false;
            const std::string endpoint = pathPosition == 0.0 ?
                "At pos1. Hold W to move forward." : "At pos2. Hold S to move backward.";
            if (hudStatus != endpoint) {
                hudStatus = endpoint;
                glutPostRedisplay();
            }
        }
    }
    glutTimerFunc(16, timer, 0);
}

void uploadMesh(const Mesh& mesh, RenderMesh& target) {
    const std::size_t vertexBytes = mesh.vertices.size() * sizeof(Vertex);
    const std::size_t indexBytes = mesh.indices.size() * sizeof(GLuint);
    if (vertexBytes > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) ||
        indexBytes > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) ||
        mesh.indices.size() > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max())) {
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

GLuint loadTexture(const std::filesystem::path& path) {
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

} // namespace

int main(int argc, char** argv) {
    try {
        cameraFileDirectory = std::filesystem::absolute(argv[0]).parent_path();
        loadCameraPath();
        initializeParticles();
        Mesh model = loadEmbeddedMesh();
        Mesh adrian = loadAdrianMesh();
        const Vec3 placed = adrianSceneCenter();
        for (Vertex& vertex : adrian.vertices) {
            vertex.x = static_cast<GLfloat>(placed.x +
                (vertex.x - adrian_data::centerX) * adrianScale);
            vertex.y = static_cast<GLfloat>(placed.y +
                (vertex.y - adrian_data::centerY) * adrianScale);
            vertex.z = static_cast<GLfloat>(placed.z +
                (vertex.z - adrian_data::centerZ) * adrianScale);
        }
        std::cout << "Embedded scene: " << model.indices.size() / 3 << " model triangles and "
                  << adrian.indices.size() / 3 << " Adrian triangles\n";

        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
        glutInitWindowSize(900, 700);
        glutInitWindowPosition(100, 100);
        glutCreateWindow("Embedded Scene - W/S camera path, 3 autoplay");
        if (glewInit() != GLEW_OK) throw std::runtime_error("GLEW initialization failed");

        reshape(900, 700);
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

        uploadMesh(model, modelMesh);
        uploadMesh(adrian, adrianMesh);
        stbi_set_flip_vertically_on_load(1);
        adrianTexture = loadTexture(cameraFileDirectory / "adrian_texture_map.jpg");
        redAdrianTexture = loadTexture(cameraFileDirectory / "red_adrian_texture.png");

        std::cout << "Controls: hold W/S to move between pos1 and pos2; "
                     "3 to autoplay; Esc to exit\n";
        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glutKeyboardFunc(keyboardDown);
        glutKeyboardUpFunc(keyboardUp);
        glutMouseFunc(mouseButton);
        glutIgnoreKeyRepeat(1);
        lastFrameTime = glutGet(GLUT_ELAPSED_TIME);
        glutTimerFunc(16, timer, 0);
        glutMainLoop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
