#include <cmath>
#define M_PI 3.14159265358979323846
#include <xmemory>
class Camera {
    double theta, phi;
    double posX, posY, posZ;
    double speed;

public:
    Camera() : theta(0.78), phi(0), posX(-140), posY(-185), posZ(-140), speed(2.0) {}

    double getDirX() { return cos(phi) * sin(theta); }
    double getDirY() { return sin(phi); }
    double getDirZ() { return cos(phi) * cos(theta); }

    double getX() { return posX; }
    double getY() { return posY; }
    double getZ() { return posZ; }

    void moveRight() { theta += 0.05; }
    void moveLeft() { theta -= 0.05; }
    void moveUp() { phi += 0.05; if (phi > 1.4) phi = 1.4; }
    void moveDown() { phi -= 0.05; if (phi < -1.4) phi = -1.4; }

    void walkForward() {
        posX += getDirX() * speed;
        posZ += getDirZ() * speed;
    }
    void walkBackward() {
        posX -= getDirX() * speed;
        posZ -= getDirZ() * speed;
    }
    void moveGlobalUp() {
        posY += speed;
    }

    void moveGlobalDown() {
        posY -= speed;
        if (posY < -198.0f) posY = -198.0f;
    }
};