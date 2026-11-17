#include "scene.h"
#include <GL/freeglut.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <fstream>
#include <sstream>

namespace scene {

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

} // namespace scene
