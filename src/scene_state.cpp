#include "scene.h"

namespace scene {

RenderMesh modelMesh, adrianMesh;
GLuint adrianTexture = 0, redAdrianTexture = 0;
const Vec3 initialCamera{0.0, 0.0, 1.6 + adrianModelZOffset};
Vec3 camera = initialCamera;
Vec3 particleAnchor = initialCamera;
double moveSpeed = shipMoveSpeed;
double cameraYaw = 0.0, cameraPitch = 0.0;
double particleYaw = 0.0, particlePitch = 0.0;
std::array<bool, 256> keys{};
int lastFrameTime = 0;
int windowWidth = 900, windowHeight = 700;
std::filesystem::path cameraFileDirectory, textureFileDirectory;
std::string hudStatus = "Loading pos1.csv and pos2.csv...";
CameraState animationStart, animationEnd;
bool pathReady = false, animating = false;
double pathPosition = 0.0, pathDistance = 0.0;
std::array<Particle, 30000> particles{};

} // namespace scene
