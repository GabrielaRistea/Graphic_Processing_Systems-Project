#include <glew.h>
#include <freeglut.h>
#include <iostream>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Camera.h"

GLuint grassTexture;
GLuint skybox[6];

float cameraX = 0;
float cameraY = 5;
float cameraZ = 20;
Camera camera;

GLuint loadTexture(const char* file)
{
    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(file, &width, &height, &channels, 0);

    if (!data)
    {
        std::cout << "Nu s-a incarcat textura\n";
        return 0;
    }

    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum format = channels == 4 ? GL_RGBA : GL_RGB;

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return texture;
}

void drawSkybox(float size)
{
    glDepthMask(GL_FALSE);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    // RIGHT
    glBindTexture(GL_TEXTURE_2D, skybox[0]);
    glBegin(GL_QUADS);
    /*glTexCoord2f(0, 0); glVertex3f(size, -size, -size);
    glTexCoord2f(1, 0); glVertex3f(size, -size, size);
    glTexCoord2f(1, 1); glVertex3f(size, size, size);
    glTexCoord2f(0, 1); glVertex3f(size, size, -size);*/
    glTexCoord2f(0, 0.4f); glVertex3f(size, -size, -size); // 0.5 înseamnă jumătatea pozei
    glTexCoord2f(1, 0.4f); glVertex3f(size, -size, size);
    glTexCoord2f(1, 1.0f); glVertex3f(size, size, size);   // 1.0 înseamnă sus de tot
    glTexCoord2f(0, 1.0f); glVertex3f(size, size, -size);
    glEnd();

    // LEFT
    glBindTexture(GL_TEXTURE_2D, skybox[1]);
    glBegin(GL_QUADS);
    /*glTexCoord2f(0, 0); glVertex3f(-size, -size, size);
    glTexCoord2f(1, 0); glVertex3f(-size, -size, -size);
    glTexCoord2f(1, 1); glVertex3f(-size, size, -size);
    glTexCoord2f(0, 1); glVertex3f(-size, size, size);*/
    glTexCoord2f(0, 0.4f); glVertex3f(-size, -size, size);
    glTexCoord2f(1, 0.4f); glVertex3f(-size, -size, -size);
    glTexCoord2f(1, 1.0f); glVertex3f(-size, size, -size);
    glTexCoord2f(0, 1.0f); glVertex3f(-size, size, size);
    glEnd();

    // TOP
    glBindTexture(GL_TEXTURE_2D, skybox[2]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-size, size, -size);
    glTexCoord2f(1, 0); glVertex3f(size, size, -size);
    glTexCoord2f(1, 1); glVertex3f(size, size, size);
    glTexCoord2f(0, 1); glVertex3f(-size, size, size);
    glEnd();

    // BOTTOM
    glBindTexture(GL_TEXTURE_2D, skybox[3]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-size, -size, size);
    glTexCoord2f(1, 0); glVertex3f(size, -size, size);
    glTexCoord2f(1, 1); glVertex3f(size, -size, -size);
    glTexCoord2f(0, 1); glVertex3f(-size, -size, -size);
    glEnd();

    // FRONT
    glBindTexture(GL_TEXTURE_2D, skybox[4]);
    glBegin(GL_QUADS);
    /*glTexCoord2f(0, 0); glVertex3f(-size, -size, -size);
    glTexCoord2f(1, 0); glVertex3f(size, -size, -size);
    glTexCoord2f(1, 1); glVertex3f(size, size, -size);
    glTexCoord2f(0, 1); glVertex3f(-size, size, -size);*/
    glTexCoord2f(0, 0.4f); glVertex3f(-size, -size, -size);
    glTexCoord2f(1, 0.4f); glVertex3f(size, -size, -size);
    glTexCoord2f(1, 1.0f); glVertex3f(size, size, -size);
    glTexCoord2f(0, 1.0f); glVertex3f(-size, size, -size);
    glEnd();

    // BACK
    glBindTexture(GL_TEXTURE_2D, skybox[5]);
    glBegin(GL_QUADS);
    /*glTexCoord2f(0, 0); glVertex3f(size, -size, size);
    glTexCoord2f(1, 0); glVertex3f(-size, -size, size);
    glTexCoord2f(1, 1); glVertex3f(-size, size, size);
    glTexCoord2f(0, 1); glVertex3f(size, size, size);*/
    glTexCoord2f(0, 0.4f); glVertex3f(size, -size, size);
    glTexCoord2f(1, 0.4f); glVertex3f(-size, -size, size);
    glTexCoord2f(1, 1.0f); glVertex3f(-size, size, size);
    glTexCoord2f(0, 1.0f); glVertex3f(size, size, size);
    glEnd();

    glDepthMask(GL_TRUE);
}

void drawGround()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    float size = 50;

    glBegin(GL_QUADS);

    glTexCoord2f(0, 0); glVertex3f(-size, 0, -size);
    glTexCoord2f(5, 0); glVertex3f(size, 0, -size);
    glTexCoord2f(5, 5); glVertex3f(size, 0, size);
    glTexCoord2f(0, 5); glVertex3f(-size, 0, size);

    glEnd();
}

void drawTerrain()
{
    float groundLevel = -199.9f;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glColor3f(1.0f, 1.0f, 1.0f);
    for (float x = -200; x < 0; x += 2.0f)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for (float z = -200; z <= 0; z += 2.0f)
        {
            /*auto getY = [](float x, float z) {
                float hills = (sin(x * 0.1f) + cos(z * 0.1f) + 2.0f) * 4.0f;

                float factorX = (x < -50) ? 1.0f : abs(x) / 50.0f;
                float factorZ = (z < -50) ? 1.0f : abs(z) / 50.0f;

                return hills * factorX * factorZ;
                };*/

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

            glTexCoord2f(x / 5.0f, z / 5.0f);
            glVertex3f(x, y1, z);

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

    for (float x = -195; x < -5; x += 8.0f) {
        for (float z = -195; z < -5; z += 8.0f) {

            /*float hills = (sin(x * 0.1f) + cos(z * 0.1f) + 2.0f) * 4.0f;
            float factorX = (x < -50) ? 1.0f : abs(x) / 50.0f;
            float factorZ = (z < -50) ? 1.0f : abs(z) / 50.0f;

            float h = hills * factorX * factorZ;*/

            float hills = (sin(x * 0.1f) + cos(z * 0.1f) + 2.0f) * 4.0f;

            float factorX1 = (x > -50.0f) ? abs(x) / 50.0f : 1.0f;
            float factorX2 = (x < -150.0f) ? (x + 200.0f) / 50.0f : 1.0f;

            float factorZ1 = (z > -50.0f) ? abs(z) / 50.0f : 1.0f;
            float factorZ2 = (z < -150.0f) ? (z + 200.0f) / 50.0f : 1.0f;

            float h = hills * factorX1 * factorX2 * factorZ1 * factorZ2;


            //float h = (sin(x * 0.1f) + cos(z * 0.1f)) * 4.0f;

            drawPinkFlower(x, groundLevel + h, z);
        }
    }

    glEnable(GL_TEXTURE_2D);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    double eyeX = camera.getX();
    double eyeY = camera.getY();
    double eyeZ = camera.getZ();

    double lookX = eyeX + camera.getDirX();
    double lookY = eyeY + camera.getDirY();
    double lookZ = eyeZ + camera.getDirZ();

    gluLookAt(eyeX, eyeY, eyeZ,
        lookX, lookY, lookZ,
        0.0, 1.0, 0.0);

    glPushMatrix();
    glTranslatef(eyeX, 0.0f, eyeZ);
    drawSkybox(200);
    glPopMatrix();
    drawTerrain();
    plantFlowers();
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'd': case 'D': camera.walkForward(); break;
    case 'a': case 'A': camera.walkBackward(); break;
    case 'w': case 'W': camera.moveGlobalUp(); break;   
    case 's': case 'S': camera.moveGlobalDown(); break;
    }
    glutPostRedisplay();
}

void special(int key, int, int)
{
    switch (key)
    {
    case GLUT_KEY_LEFT: camera.moveLeft(); break;
    case GLUT_KEY_RIGHT: camera.moveRight(); break;
    case GLUT_KEY_UP: camera.moveUp(); break;
    case GLUT_KEY_DOWN: camera.moveDown(); break;
    }
    glutPostRedisplay();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(60, (float)w / h, 0.1, 500);

    glMatrixMode(GL_MODELVIEW);
}

void init()
{
    glEnable(GL_DEPTH_TEST);

    grassTexture = loadTexture("sh_dn.png");

    skybox[0] = loadTexture("sh_rt.png");
    skybox[1] = loadTexture("sh_lf.png");
    skybox[2] = loadTexture("sh_up.png");
    skybox[3] = loadTexture("sh_dn.png");
    skybox[4] = loadTexture("sh_ft.png");
    skybox[5] = loadTexture("sh_bk.png");
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 800);

    glutCreateWindow("OpenGL Scene");

    glewInit();

    init();

    glutDisplayFunc(display);

    glutReshapeFunc(reshape);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}