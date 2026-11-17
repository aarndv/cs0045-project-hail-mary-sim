#pragma once

#include <GL/glew.h>

#include <array>
#include <filesystem>
#include <string>

namespace scene {

struct Vec3 {
    double x = 0.0, y = 0.0, z = 0.0;
};

struct RenderMesh {
    GLuint vertexBuffer = 0;
    GLuint indexBuffer = 0;
    GLsizei indexCount = 0;
};

constexpr double shipMoveSpeed = 0.30;

struct CameraState {
    Vec3 position;
    double yaw = 0.0, pitch = 0.0, speed = shipMoveSpeed;
};

struct Particle {
    Vec3 position;
    float phase = 0.0f, threshold = 0.0f, tint = 1.0f, crowding = 0.0f;
    float route = 0.0f, haloScale = 1.0f;
};

extern const GLfloat adrianModelZOffset;
extern RenderMesh modelMesh, adrianMesh;
extern GLuint adrianTexture, redAdrianTexture;
extern Vec3 camera, particleAnchor;
extern double moveSpeed, cameraYaw, cameraPitch, particleYaw, particlePitch;
extern std::array<bool, 256> keys;
extern int lastFrameTime, windowWidth, windowHeight;
extern std::filesystem::path cameraFileDirectory, textureFileDirectory;
extern std::string hudStatus;
extern CameraState animationStart, animationEnd;
extern bool pathReady, animating;
extern double pathPosition, pathDistance;
extern std::array<Particle, 30000> particles;

Vec3 forwardDirection();
void loadCameraPath();
void applyPathPosition();
void startAnimation();
void initializeParticles();
void initializeSceneMeshes();
void initializeSceneTextures();
void drawParticles();
void reshape(int width, int height);
void display();
void keyboardDown(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void mouseButton(int button, int state, int x, int y);
void timer(int value);

} // namespace scene
