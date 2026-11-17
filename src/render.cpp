#include "scene.h"
#include "mesh_types.h"
#include <GL/freeglut.h>
#include <GL/glu.h>

#include <algorithm>
#include <cmath>
#include <string>

#define BUFFER_OFFSET(i) ((void*)(i))

static_assert(sizeof(Vertex) == 8 * sizeof(GLfloat), "Vertex must be tightly interleaved");

namespace scene {

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

void hudText(float x, float y, const std::string& line, void* font) {
    glRasterPos2f(x, y);
    for (unsigned char letter : line) glutBitmapCharacter(font, letter);
}

void hudRect(float left, float bottom, float right, float top) {
    glBegin(GL_QUADS);
    glVertex2f(left, bottom);
    glVertex2f(right, bottom);
    glVertex2f(right, top);
    glVertex2f(left, top);
    glEnd();
}

bool hudPlayButtonHit(int x, int y) {
    const int right = std::min(windowWidth - 16, 468);
    return pathReady && x >= right - 112 && x < right - 16 && y >= 56 && y < 82;
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

    const float left = 16.0f;
    const float right = static_cast<float>(std::min(windowWidth - 16, 468));
    const float top = static_cast<float>(windowHeight - 16);
    const float bottom = top - 143.0f;
    const float red = static_cast<float>(std::clamp((pathPosition - 0.40) / 0.07, 0.0, 1.0));
    const float accentR = 0.26f + 0.68f * red;
    const float accentG = 0.84f - 0.43f * red;
    const float accentB = 0.68f - 0.24f * red;

    glColor3f(0.004f, 0.007f, 0.012f);
    hudRect(left + 4, bottom - 4, right + 4, top - 4);
    glBegin(GL_QUADS);
    glColor3f(0.010f + 0.015f * red, 0.018f, 0.027f);
    glVertex2f(left, bottom);
    glVertex2f(right, bottom);
    glColor3f(0.026f + 0.022f * red, 0.037f, 0.050f);
    glVertex2f(right, top);
    glVertex2f(left, top);
    glEnd();
    glColor3f(accentR, accentG, accentB);
    hudRect(left, top - 3.0f, right, top);

    glColor3f(0.94f, 0.97f, 0.98f);
    hudText(left + 16, top - 33, "CAMERA PATH", GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.60f, 0.68f, 0.73f);
    hudText(right - 137, top - 30, "POS1  >  POS2", GLUT_BITMAP_8_BY_13);

    const std::string status = !pathReady ? "Camera path unavailable" :
        animating ? "Playing camera path" :
        pathPosition <= 0.0 ? "Ready at pos1" :
        pathPosition >= 1.0 ? "Arrived at pos2" :
        keys['w'] && !keys['s'] ? "Moving toward pos2" :
        keys['s'] && !keys['w'] ? "Moving toward pos1" : "Paused on camera path";
    glColor3f(0.84f, 0.91f, 0.93f);
    hudText(left + 16, top - 61, status, GLUT_BITMAP_9_BY_15);

    glColor3f(0.055f + 0.18f * accentR, 0.055f + 0.18f * accentG,
              0.065f + 0.18f * accentB);
    hudRect(right - 112, top - 66, right - 16, top - 40);
    glColor3f(pathReady ? 0.95f : 0.50f, pathReady ? 0.98f : 0.56f,
              pathReady ? 0.98f : 0.60f);
    hudText(right - (pathReady && pathPosition > 0.0 ? 104 : 99), top - 58,
            !pathReady ? "NO PATH" : pathPosition > 0.0 ? "REPLAY [3]" : "PLAY [3]",
            GLUT_BITMAP_8_BY_13);

    glColor3f(accentR, accentG, accentB);
    hudText(left + 16, top - 86,
            pathPosition >= 0.40 ? "RED FIELD ACTIVE" : "RED FIELD AT 40%",
            GLUT_BITMAP_8_BY_13);
    const int percent = pathReady ? static_cast<int>(pathPosition * 100.0 + 0.5) : 0;
    hudText(right - 60, top - 86, std::to_string(percent) + "%", GLUT_BITMAP_8_BY_13);

    glColor3f(0.11f, 0.15f, 0.18f);
    hudRect(left + 16, top - 105, right - 16, top - 98);
    glColor3f(accentR, accentG, accentB);
    hudRect(left + 16, top - 105,
            left + 16 + (right - left - 32) * static_cast<float>(pathPosition), top - 98);
    glColor3f(0.88f, 0.93f, 0.94f);
    const float marker = left + 16 + (right - left - 32) * 0.40f;
    hudRect(marker - 1, top - 107, marker + 1, top - 96);

    glColor3f(0.68f, 0.76f, 0.79f);
    hudText(left + 16, top - 127,
            pathReady ? "Hold W/S: move  |  3: autoplay  |  Esc: exit" :
                        "Check assets/camera/pos1.csv and pos2.csv",
            GLUT_BITMAP_8_BY_13);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void display() {
    const float redMix = static_cast<float>(std::clamp((pathPosition - 0.40) / 0.07, 0.0, 1.0));
    const float spawn = static_cast<float>(std::clamp((pathPosition - 0.40) / 0.60, 0.0, 1.0));
    const float particleAmount = 0.60f * spawn + 0.40f * std::max(0.0f, 2.0f * spawn - 1.0f);
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
    glTranslatef(0.0f, 0.0f, adrianModelZOffset);
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

} // namespace scene
