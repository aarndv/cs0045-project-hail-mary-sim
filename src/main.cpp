#include "scene.h"
#include <GL/freeglut.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace scene {

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
        glutPostRedisplay();
    }
}

void keyboardUp(unsigned char key, int, int) {
    keys[movementKey(key)] = false;
    glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && hudPlayButtonHit(x, y))
        startAnimation();
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
            glutPostRedisplay();
        }
    }
    glutTimerFunc(16, timer, 0);
}

} // namespace scene

int main(int argc, char** argv) {
    using namespace scene;
    try {
        const auto projectRoot = std::filesystem::absolute(argv[0]).parent_path().parent_path();
        cameraFileDirectory = projectRoot / "assets" / "camera";
        textureFileDirectory = projectRoot / "assets" / "textures";
        loadCameraPath();
        initializeParticles();
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

        initializeSceneMeshes();
        initializeSceneTextures();

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
