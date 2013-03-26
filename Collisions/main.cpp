
#include <GLUT/glut.h>
#include <sys/time.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#define window_width  1000
#define window_height 1000
#define gravity 0.5


// typedef simplifies declarations for pointer types

// TVector has a x, y and z float
typedef struct {
    double x;
    double y;
    double z;
}TVector;

// TObject3D has position and velocity TVectors
typedef struct {
    TVector oldposition;
    TVector position;
    TVector oldvelocity;
    TVector velocity;
}TObject3D;

int         posmax = 20;
int         posmin = -20;
int         velmax = 5;
int         velmin = -5;
char *      theProgramTitle;
bool        isAnimating = true;
GLuint      currentTime;
GLuint      oldTime;
int         numballs = 1;
TObject3D * balls = new TObject3D[numballs];


// render delay 100 milliseconds
const GLuint ANIMATION_DELAY = 40;

// Main loop
GLuint timeGetTime() {
    timeval time;
    gettimeofday(&time, NULL);
    return GLuint(time.tv_sec * 1000 + time.tv_usec / 1000);
}

void initialize() {
    for (int i = 0; i < numballs; i++) {
//        balls[i].position = {static_cast<double>(rand() % (posmax - posmin) + posmin), static_cast<double>(rand() % (posmax - posmin) + posmin), static_cast<double>(rand() % (posmax - posmin) + posmin)};
//        balls[i].velocity = {static_cast<double>(rand() % (velmax - velmin) + velmin), static_cast<double>(rand() % (velmax - velmin) + velmin), static_cast<double>(rand() % (velmax - velmin) + velmin)};
        balls[i].position = {0.0,0.0,0.0};
        balls[i].velocity = {0.0,0.0,0.0};
    }
    oldTime = timeGetTime();
}

void fall(double dt) {
    for (int i = 0; i < numballs; i++) {
        double newdt = dt / 100;
        double oldvel = balls[i].velocity.y;
        balls[i].position.x = balls[i].position.x + balls[i].velocity.x * newdt;
        balls[i].position.y = balls[i].position.y + balls[i].velocity.y * newdt;
        balls[i].position.z = balls[i].position.z + balls[i].velocity.z * newdt;
        balls[i].velocity.y = balls[i].velocity.y - gravity * newdt;
        std::cout << std::endl << "time" << newdt << std::endl;
        std::cout << "velocity " << balls[i].velocity.y << std::endl;
        std::cout << "position " << balls[i].position.y << std::endl;
        if (oldvel < 0) {
            std::cout << "I'M FALLING" << std::endl;
        }
    }
}

void bounce() {
    for (int i = 0; i < numballs; i++) {
        // -window_height/20 + 15 = -35
        if (balls[i].position.y <= -window_height/20 + 15 || balls[i].position.y >= window_height/20 - 15) {
            balls[i].velocity.y = -balls[i].velocity.y;
            std::cout << std:: endl << "FUCK IT TURNED AROUND" << std::endl;
        }
        std::cout << "turnaround velocity " << balls[i].velocity.y << std::endl;
        if (balls[i].position.x <= -window_width/20 + 15 || balls[i].position.x >= window_width/20 - 15) {
            balls[i].velocity.x = -balls[i].velocity.x;
        }
        if (balls[i].position.z <= -50 || balls[i].position.z >= 30) {
            balls[i].velocity.z = -balls[i].velocity.z;
        }
    }
}

//int collision() {
//    for (int i = 0; i < numballs; i++) {
//        for (int j = i + 1; j < numballs; j++) {
//            <#statements#>
//        }
//    }
//}

void makeball(TObject3D ball) {
    // Draw a sphere
    glPushMatrix();
    glTranslatef(ball.position.x, ball.position.y, ball.position.z);
    glutSolidSphere(1, 30, 30);
    glPopMatrix();
}

void display() {
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Load identity matrix
    glLoadIdentity();
    
    // Move back
    glTranslatef(0, 0, -90);
    
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat light_ambient[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat light_position[] = {-1.0, 1.0, -1.0, 0.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    
    for (int i = 0; i < numballs; i++) {
        makeball(balls[i]);
    }
    
    // Swap buffers (color buffers, makes previous render visible)
    glutSwapBuffers();
}

void processNormalKeys(unsigned char key, int x, int y) {
}

void processSpecialKeys(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_UP :
            break;
	}
}

void idle () {
    if (isAnimating) {
        currentTime = timeGetTime();
        if ((currentTime - oldTime) > ANIMATION_DELAY) {
            // move the balls
            fall(currentTime - oldTime);
            bounce();
//            collision();
            // compute the frame rate
            oldTime = currentTime;
        }
        
        // notify window it has to be repainted
        glutPostRedisplay();
    }
    
}

// Initialze OpenGL perspective matrix
void GL_Setup(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, window_width, window_height);
    gluPerspective(45, (float) width / height, 1, 1000);
    glMatrixMode(GL_MODELVIEW);
}

// Initialize GLUT and start main loop
int main(int argc, char** argv) {
    
    // Set up initial positions and velocities for each ball
    initialize();
    
    glutInit(&argc, argv);
    glutInitWindowSize(window_width, window_height);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutCreateWindow("Falling Ball");
    
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    
    GL_Setup(window_width, window_height);
    
    glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKeys);
    
    glutMainLoop();
}
