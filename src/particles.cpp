#include "scene.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace scene {

void initializeParticles() {
    if (!pathReady) return;
    std::mt19937 random(2026);
    std::mt19937 haloRandom(2031);
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    for (std::size_t i = 0; i < particles.size(); ++i) {
        Particle& particle = particles[i];
        const bool lateField = i >= 18000;
        const bool sideField = i >= 24000;
        const double angle = sideField ? (unit(random) * 2.0 - 1.0) * 0.80 +
            (unit(random) < 0.5 ? 0.0 : 3.141592653589793) :
            unit(random) * 6.283185307179586;
        // Keep the outer volume circular while favoring its sides over the center.
        const double fieldRadius = i % 20 == 0 ? 0.70 : (i % 20 < 7 ? 1.50 : 2.45);
        const double radius = sideField ? 1.40 + unit(random) * 0.80 :
            std::sqrt(unit(random)) * fieldRadius;
        particle.position = {std::cos(angle) * radius,
                             std::sin(angle) * radius,
                             sideField ? 0.70 + unit(random) * 1.0 :
                             i % 47 == 0 ? 0.065 + unit(random) * 0.065 :
                             i % 31 == 0 ? 0.08 + unit(random) * 0.20 :
                                          0.35 + unit(random) * 3.30};
        particle.phase = static_cast<float>(unit(random) * 6.283185307179586);
        particle.threshold = static_cast<float>((lateField ? 0.50 : 0.0) +
            unit(random) * (lateField ? 0.50 : 1.0));
        particle.tint = static_cast<float>(0.65 + unit(random) * 0.35);
        particle.route = static_cast<float>(sideField ? 0.82 + unit(random) * 0.13 :
            lateField ? 0.58 + unit(random) * 0.40 : unit(random) * 0.90);
        if (unit(haloRandom) < 0.08)
            particle.haloScale = static_cast<float>(1.20 + unit(haloRandom) * 1.30);
    }
    // Nearby projected particles share similar screen positions along the path.
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
    const double endFade = std::clamp((pathPosition - 0.75) / 0.25, 0.0, 1.0);
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
        if (depth <= 0.055) continue;
        const double farFade = std::clamp((depth - 1.2) / 2.5, 0.0, 1.0);
        const double visibility = 1.0 - 0.90 * endFade * farFade;
        if (particle.threshold > density * visibility) continue;
        const double screenX = (offset.x * viewRight.x + offset.y * viewRight.y +
                                offset.z * viewRight.z) / (depth * 0.414213562373095);
        const double screenY = (offset.x * viewUp.x + offset.y * viewUp.y +
                                offset.z * viewUp.z) / (depth * 0.414213562373095);
        if (std::abs(screenX) > static_cast<double>(windowWidth) / windowHeight + 0.25 ||
            std::abs(screenY) > 1.25) continue;
        if (screenX * screenX + screenY * screenY < 0.65 * 0.65 && i % 13 < 5) continue;
        const double pixelScale = 0.828427124746 * depth / windowHeight;
        const int sizeClass = i % 20;
        const double pixels = (sizeClass < 14 ? 4.5 : (sizeClass < 19 ? 8.0 : 14.0)) *
                              std::min(particle.position.z < 0.15 ? 4.0 :
                                       particle.position.z < 0.30 ? 3.0 : 2.0,
                                       0.7 + 0.45 / depth);
        const Vec3 color = sizeClass < 14 ? Vec3{0.75, 0.08, 0.28} :
                           (sizeClass < 19 ? Vec3{1.0, 0.28, 0.52} : Vec3{1.0, 0.70, 0.82});
        const float tint = particle.tint * (1.0f + 0.25f * particle.crowding);
        drawParticleDisk(position, viewRight, viewUp, pixels * pixelScale * particle.haloScale, color,
                         tint, static_cast<float>((0.50f + 0.35f * particle.crowding) * visibility));
        if (depth < 0.35 && particle.position.z < 0.30 && particle.phase < 2.1f)
            drawParticleDisk(position, viewRight, viewUp, pixels * pixelScale * 0.22,
                             {1.0, 0.82, 0.90}, 1.0f, 0.95f, 0.95f);
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

} // namespace scene
