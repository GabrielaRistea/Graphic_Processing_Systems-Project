#include <glew.h>
#include <freeglut.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <cstdlib>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Camera.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


GLuint grassTexture;
GLuint skybox[6];
GLuint roadTexture;
GLuint buildingTexture;

float cameraX = 0;
float cameraY = 5;
float cameraZ = 20;
Camera camera;
bool keys[256] = { false };
int windowWidth = 1000;
int windowHeight = 800;
bool followCamera = true;
float currentCamX = 60.0f;
float currentCamY = -190.0f;
float currentCamZ = 35.0f;

GLfloat light_position[] = { 0.0, 10.0, 0.0, 1.0 };

bool isShadowPass = false;
float groundPlane[] = { 0.0f, 1.0f, 0.0f, 199.8f };

struct AABB { float x, z, w, d; };
std::vector<AABB> sceneColliders;

float lamps[2][3] = { {60.0f, -199.8f, 60.0f}, {120.0f, -199.8f, 60.0f} };

float carX = 60.0f;
float carZ = 20.0f;
float carAngle = 0.0f;
float carSpeed = 0.6f;

float predefinedAngle = 0.0f;
float busX = 0.0f;
float busZ = 0.0f;

struct RandomObj { float x, y, z, dx, dy, dz; };
std::vector<RandomObj> randomObjects;

void initLogic() {
    float centerX = 60.0f, centerZ = 60.0f;
    int totalObjects = 16;
    float outerRadius = 60.0f;

    for (int i = 0; i < totalObjects; i++) {
        float angle = 2.0f * M_PI * (float)i / (float)totalObjects;
        float sx = 1.8f, sz = 1.2f;
        float xOut = centerX + (outerRadius * cos(angle) * sx);
        float zOut = centerZ + (outerRadius * sin(angle) * sz);

        if (i % 2 == 0) {
            sceneColliders.push_back({ xOut, zOut, 8.0f, 8.0f });
        }
        else {
            sceneColliders.push_back({ xOut, zOut, 2.0f, 2.0f });
        }
    }

    for (int i = 0; i < 2; i++) {
        sceneColliders.push_back({ lamps[i][0], lamps[i][2], 2.0f, 2.0f });
    }

    for (int i = 0; i < 6; i++) {
        RandomObj ro;
        ro.x = centerX + (rand() % 100 - 50);
        ro.z = centerZ + (rand() % 100 - 50);
        ro.y = -190.0f + (rand() % 15);
        ro.dx = ((rand() % 100) / 100.0f - 0.5f) * 0.4f;
        ro.dy = ((rand() % 100) / 100.0f - 0.5f) * 0.1f;
        ro.dz = ((rand() % 100) / 100.0f - 0.5f) * 0.4f;
        randomObjects.push_back(ro);
    }
}

bool isColliding(float nextX, float nextZ) {
    float carSize = 2.5f; 

    for (auto& b : sceneColliders) {
        float minX = b.x - b.w / 2.0f - carSize;
        float maxX = b.x + b.w / 2.0f + carSize;
        float minZ = b.z - b.d / 2.0f - carSize;
        float maxZ = b.z + b.d / 2.0f + carSize;

        if (nextX > minX && nextX < maxX && nextZ > minZ && nextZ < maxZ) {
            return true;
        }
    }

    float busCollisionRadius = 4.5f; 
    float distToBus = sqrt(pow(nextX - busX, 2) + pow(nextZ - busZ, 2));

    if (distToBus < (carSize + busCollisionRadius)) {
        return true; 
    }

    return false;
}

GLuint loadTexture(const char* file) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(file, &width, &height, &channels, 0);

    if (!data) {
        std::cout << "Nu s-a incarcat textura: " << file << "\n";
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return texture;
}
void drawGround(float size) {
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    //glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -1.0f, -size);
    glTexCoord2f(10.0f, 0.0f); glVertex3f(size, -1.0f, -size);
    glTexCoord2f(10.0f, 10.0f); glVertex3f(size, -1.0f, size);
    glTexCoord2f(0.0f, 10.0f); glVertex3f(-size, -1.0f, size);
    glEnd();
}

void drawSkybox(float size) {
    glDepthMask(GL_FALSE);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, skybox[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0.4f); glVertex3f(size, -size, -size);
    glTexCoord2f(1, 0.4f); glVertex3f(size, -size, size);
    glTexCoord2f(1, 1.0f); glVertex3f(size, size, size);
    glTexCoord2f(0, 1.0f); glVertex3f(size, size, -size);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, skybox[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0.4f); glVertex3f(-size, -size, size);
    glTexCoord2f(1, 0.4f); glVertex3f(-size, -size, -size);
    glTexCoord2f(1, 1.0f); glVertex3f(-size, size, -size);
    glTexCoord2f(0, 1.0f); glVertex3f(-size, size, size);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, skybox[2]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-size, size, -size);
    glTexCoord2f(1, 0); glVertex3f(size, size, -size);
    glTexCoord2f(1, 1); glVertex3f(size, size, size);
    glTexCoord2f(0, 1); glVertex3f(-size, size, size);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, skybox[3]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-size, -size, size);
    glTexCoord2f(1, 0); glVertex3f(size, -size, size);
    glTexCoord2f(1, 1); glVertex3f(size, -size, -size);
    glTexCoord2f(0, 1); glVertex3f(-size, -size, -size);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, skybox[4]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0.4f); glVertex3f(-size, -size, -size);
    glTexCoord2f(1, 0.4f); glVertex3f(size, -size, -size);
    glTexCoord2f(1, 1.0f); glVertex3f(size, size, -size);
    glTexCoord2f(0, 1.0f); glVertex3f(-size, size, -size);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, skybox[5]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0.4f); glVertex3f(size, -size, size);
    glTexCoord2f(1, 0.4f); glVertex3f(-size, -size, size);
    glTexCoord2f(1, 1.0f); glVertex3f(-size, size, size);
    glTexCoord2f(0, 1.0f); glVertex3f(size, size, size);
    glEnd();

    glDepthMask(GL_TRUE);
}


void drawFlatGround() {
    float groundLevel = -200.0f;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    float s = 1500.0f; 
    float uv = 100.0f;

    glTexCoord2f(0.0f, 0.0f); glVertex3f(-s, groundLevel, -s);
    glTexCoord2f(uv, 0.0f);   glVertex3f(s, groundLevel, -s);
    glTexCoord2f(uv, uv);     glVertex3f(s, groundLevel, s);
    glTexCoord2f(0.0f, uv);   glVertex3f(-s, groundLevel, s);
    glEnd();
}

void drawTerrain() {
    float groundLevel = -199.9f;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    for (float x = -200; x < 0; x += 2.0f) {
        glBegin(GL_TRIANGLE_STRIP);
        for (float z = -200; z < 0; z += 2.0f) {
            auto getY = [](float x, float z) {
                float hills = (sin(x * 0.1f) + cos(z * 0.1f) + 2.0f) * 4.0f;
                float factorX1 = (x > -50.0f) ? abs(x) / 50.0f : 1.0f;
                float factorX2 = (x < -150.0f) ? (x + 200.0f) / 50.0f : 1.0f;
                float factorZ1 = (z > -50.0f) ? abs(z) / 50.0f : 1.0f;
                float factorZ2 = (z < -150.0f) ? (z + 200.0f) / 50.0f : 1.0f;
                return hills * factorX1 * factorX2 * factorZ1 * factorZ2;
                };

            float y1 = groundLevel + getY(x, z);
            float y2 = groundLevel + getY(x + 2.0f, z);

            /*glTexCoord2f(x / 5.0f, z / 5.0f); glVertex3f(x, y1, z);
            glTexCoord2f((x + 2.0f) / 5.0f, z / 5.0f); glVertex3f(x + 2.0f, y2, z);*/
            glNormal3f(0.0f, 1.0f, 0.0f);
            glTexCoord2f(x / 5.0f, z / 5.0f);
            glVertex3f(x, y1, z);

            glNormal3f(0.0f, 1.0f, 0.0f);
            glTexCoord2f((x + 2.0f) / 5.0f, z / 5.0f);
            glVertex3f(x + 2.0f, y2, z);
        }
        glEnd();
    }
}




void drawPinkFlower(float x, float y, float z) {
    float size = 0.3f;
    float height = 0.6f;

    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 0.4f, 0.7f);

    glBegin(GL_QUADS);
    glVertex3f(x - size, y, z);
    glVertex3f(x + size, y, z);
    glVertex3f(x + size, y + height, z);
    glVertex3f(x - size, y + height, z);

    glVertex3f(x, y, z - size);
    glVertex3f(x, y, z + size);
    glVertex3f(x, y + height, z + size);
    glVertex3f(x, y + height, z - size);
    glEnd();

    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_POINTS);
    glVertex3f(x, y + height, z);
    glEnd();

    glEnable(GL_TEXTURE_2D);
}

void plantFlowers() {
    float groundLevel = -199.9f;
    glDisable(GL_TEXTURE_2D);

    for (float x = -200; x < 0; x += 4.0f) {
        for (float z = -200; z < 0; z += 4.0f) {
            float hills = (sin(x * 0.1f) + cos(z * 0.1f) + 2.0f) * 4.0f;
            float factorX1 = (x > -50.0f) ? abs(x) / 50.0f : 1.0f;
            float factorX2 = (x < -150.0f) ? (x + 200.0f) / 50.0f : 1.0f;
            float factorZ1 = (z > -50.0f) ? abs(z) / 50.0f : 1.0f;
            float factorZ2 = (z < -150.0f) ? (z + 200.0f) / 50.0f : 1.0f;
            float h = hills * factorX1 * factorX2 * factorZ1 * factorZ2;
            drawPinkFlower(x, groundLevel + h, z);
        }
    }
    glEnable(GL_TEXTURE_2D);
}

void drawRoad(float centerX, float centerZ, float innerRadius, float outerRadius, int segments) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, roadTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    float height = -199.8f;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * (float)i / (float)segments;
        float stretchX = 1.8f;
        float stretchZ = 1.2f;

        float xOut = centerX + (outerRadius * cos(angle) * stretchX);
        float zOut = centerZ + (outerRadius * sin(angle) * stretchZ);
        float xIn = centerX + (innerRadius * cos(angle) * stretchX);
        float zIn = centerZ + (innerRadius * sin(angle) * stretchZ);

        float tCoord = (float)i / (segments / 15.0f);

        glTexCoord2f(1.0f, tCoord); glVertex3f(xOut, height, zOut);
        glTexCoord2f(0.0f, tCoord); glVertex3f(xIn, height, zIn);
    }
    glEnd();
    glDisable(GL_CULL_FACE);
}

void drawBuilding(float x, float y, float z, float w, float h, float d) {
    if (!isShadowPass) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, buildingTexture);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.0f, 0.05f, 0.1f, 0.6f);
    }

    glPushMatrix();
    glTranslatef(x, y + h / 2.0f, z);
    glScalef(w, h, d);

    glBegin(GL_QUADS);
    // Fata
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    // Spate
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    // Sus
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    // Jos
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    // Dreapta
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    // Stanga
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
    glEnd();

    glPopMatrix();
}

void drawTree(float x, float y, float z) {
    glDisable(GL_TEXTURE_2D);

    if (!isShadowPass) glColor3f(0.4f, 0.2f, 0.1f);
    else glColor4f(0.0f, 0.05f, 0.1f, 0.6f);

    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-90, 1, 0, 0);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, 0.8f, 0.8f, 7.0f, 10, 10);
    glPopMatrix();
    gluDeleteQuadric(quad);

    if (!isShadowPass) glColor3f(0.0f, 0.5f, 0.0f);
    else glColor4f(0.0f, 0.05f, 0.1f, 0.6f);

    glPushMatrix();
    glTranslatef(x, y + 2.5f, z);
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(5.0f, 10.0f, 12, 12);
    glPopMatrix();

    if (!isShadowPass) {
        glEnable(GL_TEXTURE_2D);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
}

void drawStaticObjects() {
    float centerX = 60.0f;
    float centerZ = 60.0f;
    float groundLevel = -199.9f;
    int totalObjects = 16;
    float outerRadius = 60.0f;

    for (int i = 0; i < totalObjects; i++) {
        float angle = 2.0f * M_PI * (float)i / (float)totalObjects;

        float sx = 1.8f;
        float sz = 1.2f;
        float xOut = centerX + (outerRadius * cos(angle) * sx);
        float zOut = centerZ + (outerRadius * sin(angle) * sz);

        if (i % 2 == 0) {
            drawBuilding(xOut, groundLevel, zOut, 8.0f, 15.0f + (i * 2), 8.0f);
        }
        else {
            drawTree(xOut, groundLevel, zOut);
        }
    }
}

void drawDynamicObjects() {
    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(carX, -199.3f, carZ);
    glRotatef(carAngle * 180.0f / M_PI, 0, 1, 0);

    if (!isShadowPass) glColor3f(0.8f, 0.1f, 0.1f);
    else glColor4f(0.0f, 0.0f, 0.0f, 0.6f);

    glPushMatrix();
    glScalef(2.5f, 1.0f, 4.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);
    glScalef(1.8f, 1.0f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(busX, -199.0f, busZ);
    glRotatef(-predefinedAngle * 180.0f / M_PI, 0, 1, 0);

    if (!isShadowPass) glColor3f(0.1f, 0.3f, 0.8f);
    else glColor4f(0.0f, 0.0f, 0.0f, 0.6f);

    glScalef(3.0f, 2.5f, 7.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    for (auto& ro : randomObjects) {
        glPushMatrix();
        glTranslatef(ro.x, ro.y, ro.z);
        if (!isShadowPass) {
            float em[] = { 0.8f, 0.8f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT, GL_EMISSION, em);
            //glColor3f(1.0f, 1.0f, 0.2f);
            glColor3f(4.0f, 1.0f, 0.0f);
        }
        else {
            glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
        }
        glutSolidSphere(0.6f, 10, 10);

        float emZ[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_EMISSION, emZ);
        glPopMatrix();
    }

    glEnable(GL_TEXTURE_2D);
}

void shadowMatrix(float shadowMat[16], float groundplane[4], float lightpos[4]) {
    float dot = groundplane[0] * lightpos[0] + groundplane[1] * lightpos[1] +
        groundplane[2] * lightpos[2] + groundplane[3] * lightpos[3];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            shadowMat[j * 4 + i] = (i == j) ? (dot - lightpos[i] * groundplane[j]) : (-lightpos[i] * groundplane[j]);
        }
    }
}

void drawStreetLamp(float x, float y, float z) {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glColor3f(0.3f, 0.3f, 0.3f);
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-90, 1, 0, 0);
    GLUquadric* q = gluNewQuadric();
    gluCylinder(q, 0.5, 0.3, 50.0, 10, 10);
    gluDeleteQuadric(q);

    glTranslatef(0, 0, 50.0);
    glColor3f(1.0f, 1.0f, 0.8f);
    glutSolidSphere(1.5, 10, 10);
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    if (glIsEnabled(GL_LIGHTING)) glEnable(GL_LIGHTING);
}

void drawLightCone(float x, float y, float z) {
    float height = 50.0f;
    float radiusTop = 0.4f;
    float radiusBottom = 7.0f;
    int segments = 30;

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * (float)i / segments;
        float cx = cos(angle);
        float cz = sin(angle);

        glColor4f(1.0f, 0.9f, 0.2f, 0.4f);
        glVertex3f(x + cx * radiusTop, y + height, z + cz * radiusTop);
        glColor4f(1.0f, 0.9f, 0.2f, 0.0f);
        glVertex3f(x + cx * radiusBottom, y, z + cz * radiusBottom);
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    double eyeX, eyeY, eyeZ;
    double targetX, targetY, targetZ;

    if (followCamera) {
        float distanceBehind = 15.0f;
        float heightAbove = 7.0f;

        float targetEyeX = carX - sin(carAngle) * distanceBehind;
        float targetEyeY = -199.3f + heightAbove;
        float targetEyeZ = carZ - cos(carAngle) * distanceBehind;

        float smoothness = 0.08f;

        currentCamX += (targetEyeX - currentCamX) * smoothness;
        currentCamY += (targetEyeY - currentCamY) * smoothness;
        currentCamZ += (targetEyeZ - currentCamZ) * smoothness;

        eyeX = currentCamX;
        eyeY = currentCamY;
        eyeZ = currentCamZ;

        targetX = carX;
        targetY = -199.3f + 2.0f;
        targetZ = carZ;
    }
    else {
        eyeX = camera.getX();
        eyeY = camera.getY();
        eyeZ = camera.getZ();

        targetX = eyeX + camera.getDirX();
        targetY = eyeY + camera.getDirY();
        targetZ = eyeZ + camera.getDirZ();
    }

    gluLookAt(eyeX, eyeY, eyeZ,
        targetX, targetY, targetZ,
        0.0, 1.0, 0.0);

    glDisable(GL_LIGHTING);
    //glDisable(GL_DEPTH_TEST);
    glPushMatrix();
        glTranslatef(eyeX, 0.0f, eyeZ);
        drawSkybox(200);
    glPopMatrix();

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    //float lamps[2][3] = { {60.0f, -199.8f, 60.0f}, {120.0f, -199.8f, 60.0f} };

    drawFlatGround();
    //drawFixedFloor();
    //drawGround(200);
    drawTerrain();
    plantFlowers();
    //drawFixedFloor();
    drawRoad(60.0f, 60.0f, 40.0f, 55.0f, 100);

    drawStaticObjects();
    drawDynamicObjects(); 

    for (int i = 0; i < 2; i++) {
        drawStreetLamp(lamps[i][0], lamps[i][1], lamps[i][2]);
        drawLightCone(lamps[i][0], lamps[i][1], lamps[i][2]);
    }

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (int i = 0; i < 2; i++) {
        float lPos[] = { lamps[i][0], lamps[i][1] + 150.0f, lamps[i][2], 1.0f };
        float sMat[16];
        shadowMatrix(sMat, groundPlane, lPos);

        glPushMatrix();
        glMultMatrixf(sMat);

        isShadowPass = true;
        drawStaticObjects();
        drawDynamicObjects(); 
        isShadowPass = false;

        glPopMatrix();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glutSwapBuffers();
}

void special(int key, int, int) {
    switch (key) {
    case GLUT_KEY_LEFT: camera.moveLeft(); break;
    case GLUT_KEY_RIGHT: camera.moveRight(); break;
    case GLUT_KEY_UP: camera.moveUp(); break;
    case GLUT_KEY_DOWN: camera.moveDown(); break;
    }
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    keys[key] = true;

    if (key == 'c' || key == 'C') {
        followCamera = !followCamera;
        glutPostRedisplay();
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    keys[key] = false;
}

void updateScene() {
    if (keys['w'] || keys['W']) camera.walkForward();
    if (keys['s'] || keys['S']) camera.walkBackward();
    if (keys['a'] || keys['A']) camera.strafeLeft();
    if (keys['d'] || keys['D']) camera.strafeRight();
    if (keys['r'] || keys['R']) camera.moveGlobalUp();
    if (keys['f'] || keys['F']) camera.moveGlobalDown();
    predefinedAngle += 0.005f;
    float sx = 1.8f, sz = 1.2f;
    float r = 47.5f;
    busX = 60.0f + r * cos(predefinedAngle) * sx;
    busZ = 60.0f + r * sin(predefinedAngle) * sz;

    float nextX = carX;
    float nextZ = carZ;

    if (keys['i'] || keys['I']) {
        nextX += sin(carAngle) * carSpeed;
        nextZ += cos(carAngle) * carSpeed;
    }
    if (keys['k'] || keys['K']) {
        nextX -= sin(carAngle) * carSpeed;
        nextZ -= cos(carAngle) * carSpeed;
    }
    if (keys['j'] || keys['J']) carAngle += 0.04f;
    if (keys['l'] || keys['L']) carAngle -= 0.04f;

    if (!isColliding(nextX, nextZ)) {
        carX = nextX;
        carZ = nextZ;
    }

    //predefinedAngle += 0.005f; 

    for (auto& ro : randomObjects) {
        ro.x += ro.dx;
        ro.y += ro.dy;
        ro.z += ro.dz;

        if (ro.y < -195 || ro.y > -175) ro.dy *= -1;
        if (ro.x < 0 || ro.x > 120) ro.dx *= -1;
        if (ro.z < 0 || ro.z > 120) ro.dz *= -1;
    }

    glutPostRedisplay();
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float)w / h, 0.1, 500);
    glMatrixMode(GL_MODELVIEW);
}

void init() {
    glEnable(GL_DEPTH_TEST);

    grassTexture = loadTexture("sh_dn.png");
    skybox[0] = loadTexture("sh_rt.png");
    skybox[1] = loadTexture("sh_lf.png");
    skybox[2] = loadTexture("sh_up.png");
    skybox[3] = loadTexture("sh_dn.png");
    skybox[4] = loadTexture("sh_ft.png");
    skybox[5] = loadTexture("sh_bk.png");
    roadTexture = loadTexture("asphalt.jpg");
    buildingTexture = loadTexture("building5.jpg");

    initLogic(); 
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 800);
    glutCreateWindow("OpenGL Scene - Dynamic Elements");
    glewInit();
    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutIdleFunc(updateScene);

    glutMainLoop();
    return 0;
}

