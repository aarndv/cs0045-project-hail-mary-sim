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
Vec3 particleAnchor = initialCamera;
double moveSpeed = shipMoveSpeed;
double cameraYaw = 0.0;
double cameraPitch = 0.0;
double particleYaw = 0.0, particlePitch = 0.0;
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
    float phase = 0.0f, threshold = 0.0f, tint = 1.0f, crowding = 0.0f;
    float route = 0.0f;
};
std::array<Particle, 24000> particles{};

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
    // The field drifts only one tenth as far as the camera travels.
    particleAnchor = {animationStart.position.x + 0.10 * (camera.x - animationStart.position.x),
                      animationStart.position.y + 0.10 * (camera.y - animationStart.position.y),
                      animationStart.position.z + 0.10 * (camera.z - animationStart.position.z)};
    particleYaw = animationStart.yaw + 0.10 * yawChange * pathPosition;
    particlePitch = animationStart.pitch +
                    0.10 * (cameraPitch - animationStart.pitch);
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
    for (std::size_t i = 0; i < particles.size(); ++i) {
        Particle& particle = particles[i];
        const bool lateField = i >= 18000;
        const double angle = unit(random) * 6.283185307179586;
        // The outer volume extends beyond the camera's view at both endpoints.
        const double fieldRadius = i % 10 == 0 ? 0.70 : (i % 10 < 5 ? 1.50 : 2.45);
        const double radius = std::sqrt(unit(random)) * fieldRadius;
        particle.position = {std::cos(angle) * radius,
                             std::sin(angle) * radius,
                             i % 31 == 0 ? 0.08 + unit(random) * 0.20 :
                                          0.35 + unit(random) * 3.30};
        particle.phase = static_cast<float>(unit(random) * 6.283185307179586);
        particle.threshold = static_cast<float>((lateField ? 0.50 : 0.0) +
            unit(random) * (lateField ? 0.50 : 1.0));
        particle.tint = static_cast<float>(0.65 + unit(random) * 0.35);
        particle.route = static_cast<float>(lateField ? 0.58 + unit(random) * 0.40 :
                                                      unit(random) * 0.90);
    }
    // Screen-space neighbors stay nearly the same while this field follows the camera.
    const auto cell = [](const Particle& p) {
        const int x = std::clamp(static_cast<int>((p.position.x + 2.45) * 32.0 / 4.90), 0, 31);
        const int y = std::clamp(static_cast<int>((p.position.y + 2.45) * 32.0 / 4.90), 0, 31);
        return y * 32 + x;
    };
    std::array<int, 32 * 32> neighbors{};
    for (const Particle& particle : particles) ++neighbors[cell(particle)];
    for (Particle& particle : particles)
        particle.crowding = std::clamp((neighbors[cell(particle)] - 12) / 50.0f, 0.0f, 1.0f);
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

Vec3 animatedParticlePosition(const Particle& particle, const Vec3& forward,
                              const Vec3& right, const Vec3& up) {
    const double phase = particle.phase + pathPosition * 20.0;
    constexpr double tanHalfFov = 0.414213562373095;
    const double depth = particle.position.z;
    const double x = (particle.position.x + 0.018 * std::sin(phase)) * depth * tanHalfFov;
    const double y = (particle.position.y + 0.018 * std::cos(phase * 1.2)) * depth * tanHalfFov;
    return {particleAnchor.x + (animationEnd.position.x - animationStart.position.x) * particle.route +
                forward.x * depth + right.x * x + up.x * y,
            particleAnchor.y + (animationEnd.position.y - animationStart.position.y) * particle.route +
                forward.y * depth + right.y * x + up.y * y,
            particleAnchor.z + (animationEnd.position.z - animationStart.position.z) * particle.route +
                forward.z * depth + right.z * x + up.z * y};
}

void drawParticleDisk(const Vec3& position, const Vec3& right, const Vec3& up,
                      double radius, const Vec3& color, float tint, float opacity,
                      float rimOpacity = 0.0f) {
    constexpr double twoPi = 6.283185307179586;
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(static_cast<GLfloat>(color.x * tint), static_cast<GLfloat>(color.y * tint),
              static_cast<GLfloat>(color.z * tint), opacity);
    glVertex3f(static_cast<GLfloat>(position.x), static_cast<GLfloat>(position.y),
               static_cast<GLfloat>(position.z));
    glColor4f(static_cast<GLfloat>(color.x * tint), static_cast<GLfloat>(color.y * tint),
              static_cast<GLfloat>(color.z * tint), rimOpacity);
    for (int corner = 0; corner <= 8; ++corner) {
        const double angle = twoPi * corner / 8.0;
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
    const Vec3 viewForward = forwardDirection();
    const Vec3 viewRight{std::cos(cameraYaw), 0.0, std::sin(cameraYaw)};
    const Vec3 viewUp{viewRight.y * viewForward.z - viewRight.z * viewForward.y,
                      viewRight.z * viewForward.x - viewRight.x * viewForward.z,
                      viewRight.x * viewForward.y - viewRight.y * viewForward.x};
    const Vec3 forward{std::sin(particleYaw) * std::cos(particlePitch),
                       std::sin(particlePitch),
                       -std::cos(particleYaw) * std::cos(particlePitch)};
    const Vec3 right{std::cos(particleYaw), 0.0, std::sin(particleYaw)};
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
        const Vec3 position = animatedParticlePosition(particle, forward, right, up);
        const Vec3 offset{position.x - camera.x, position.y - camera.y,
                          position.z - camera.z};
        const double depth = offset.x * viewForward.x + offset.y * viewForward.y +
                             offset.z * viewForward.z;
        if (depth <= 0.06) continue;
        const double screenX = (offset.x * viewRight.x + offset.y * viewRight.y +
                                offset.z * viewRight.z) / (depth * 0.414213562373095);
        const double screenY = (offset.x * viewUp.x + offset.y * viewUp.y +
                                offset.z * viewUp.z) / (depth * 0.414213562373095);
        if (std::abs(screenX) > static_cast<double>(windowWidth) / windowHeight + 0.1 ||
            std::abs(screenY) > 1.1) continue;
        const double wing = (screenY + 0.55) / 0.17;
        const double shipWidth = 0.26 + 0.28 * std::max(0.0, -screenY) +
                                 0.35 * std::exp(-wing * wing);
        const double shipFade = std::clamp(std::min((0.10 - screenY) / 0.12,
            (shipWidth - std::abs(screenX)) / 0.10), 0.0, 1.0);
        if (depth < 2.6 && (shipFade > 0.65 || particle.threshold < shipFade)) continue;
        const double pixelScale = 0.828427124746 * depth / windowHeight;
        const int sizeClass = i % 20;
        const double pixels = (sizeClass < 14 ? 4.5 : (sizeClass < 19 ? 8.0 : 14.0)) *
                              std::min(particle.position.z < 0.30 ? 3.0 : 2.0,
                                       0.7 + 0.45 / depth);
        const Vec3 color = sizeClass < 14 ? Vec3{0.75, 0.08, 0.28} :
                           (sizeClass < 19 ? Vec3{1.0, 0.28, 0.52} : Vec3{1.0, 0.70, 0.82});
        const float tint = particle.tint * (1.0f + 0.25f * particle.crowding);
        drawParticleDisk(position, viewRight, viewUp, pixels * pixelScale, color,
                         tint, 0.50f + 0.35f * particle.crowding);
        if (particle.position.z < 0.30 && particle.phase < 2.1f)
            drawParticleDisk(position, viewRight, viewUp, pixels * pixelScale * 0.22,
                             {1.0, 0.82, 0.90}, 1.0f, 0.95f, 0.95f);
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void display() {
    const float redMix = static_cast<float>(std::clamp((pathPosition - 0.40) / 0.07, 0.0, 1.0));
    const float spawn = static_cast<float>(std::clamp((pathPosition - 0.40) / 0.60, 0.0, 1.0));
    const float particleAmount = 0.75f * spawn + 0.25f * std::max(0.0f, 2.0f * spawn - 1.0f);
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
    const GLfloat topReflectionDiffuse[] = {0.04f + 0.26f * redMix + 0.32f * particleAmount,
                                            0.24f - 0.15f * redMix,
                                            0.056f + 0.064f * redMix, 1.0f};
    const GLfloat topReflectionSpecular[] = {0.024f + 0.18f * redMix + 0.22f * particleAmount,
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
    const GLfloat shipEmission[] = {0.18f * particleAmount, 0.006f * particleAmount,
                                    0.030f * particleAmount, 1.0f};
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
