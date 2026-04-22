#include <cmath>
#define M_PI 3.14159265358979323846
#include <xmemory>
class Camera {
    double theta, phi;
    double posX, posY, posZ;
    double speed;

public:
    Camera() : theta(0), phi(0), posX(0), posY(-198), posZ(0), speed(0.5) {}

    double getDirX() { return sin(theta); }
    double getDirY() { return sin(phi); }
    double getDirZ() { return cos(theta); }

    double getX() { return posX; }
    double getY() { return posY; }
    double getZ() { return posZ; }

    void moveRight() { theta += 0.05; }
    void moveLeft() { theta -= 0.05; }
    void moveUp() { phi += 0.05; if (phi > 1.4) phi = 1.4; }
    void moveDown() { phi -= 0.05; if (phi < -1.4) phi = -1.4; }

   
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
   

    void strafeLeft() {
        posX += cos(theta) * speed;
        posZ -= sin(theta) * speed;
    }

    void strafeRight() {
        posX -= cos(theta) * speed;
        posZ += sin(theta) * speed;
    }
    void walkForward() {
        posX += getDirX() * speed;
        posY += getDirY() * speed; 
        posZ += getDirZ() * speed;
    }
};



