
#include <GLUT/glut.h>
#include <sys/time.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#define window_width  1000
#define window_height 1000
#define gravity 0.1

// typedef simplifies declarations for pointer types

// TVector has a x, y and z float
typedef struct {
    float x;
    float y;
    float z;
}
TVector;

// TObject3D has position and velocity TVectors
typedef struct {
    TVector position;
    TVector velocity;
}TObject3D;

char *      theProgramTitle;
bool        isAnimating = true;
GLuint      currentTime;
GLuint      oldTime;
int         numballs = 2;
TObject3D * balls = new TObject3D[numballs];


// render delay 100 milliseconds
const GLuint ANIMATION_DELAY = 40;

// Main loop
GLuint timeGetTime()
{
    timeval time;
    gettimeofday(&time, NULL);
    return GLuint(time.tv_sec * 1000 + time.tv_usec / 1000);
}

void fall(float dt, TObject3D ball){
    float newdt = dt / 100;
    ball.velocity.y = ball.velocity.y - gravity * newdt;
    ball.position.x = ball.position.x + ball.velocity.x * newdt;
    ball.position.y = ball.position.y + ball.velocity.y * newdt;
    ball.position.z = ball.position.z + ball.velocity.z * newdt;
}

void bounce(TObject3D ball){
    if (ball.position.y < -window_height/20 + 10 || ball.position.y > window_height/20 - 10) {
        ball.velocity.y = -ball.velocity.y;
    }
    if (ball.position.x < -window_width/20 + 10 || ball.position.x > window_width/20 - 10) {
        ball.velocity.x = -ball.velocity.x;
    }
    if (ball.position.z < 0 || ball.position.z > window_width/40) {
        ball.velocity.z = -ball.velocity.z;
    }
}


void makeball(TObject3D ball) {
    // Draw a sphere
    glPushMatrix();
    glTranslatef(ball.position.x, ball.position.y, ball.position.z);
    glutSolidSphere(2, 30, 30);
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


void idle ()
{
    if (isAnimating)
    {
        currentTime = timeGetTime();
        if ((currentTime - oldTime) > ANIMATION_DELAY) {
            // move the balls
            for (int i = 0; i < numballs; i++) {
                bounce(balls[i]);
                fall(currentTime - oldTime, balls[i]);
            }
            
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
    gluPerspective(60, (float) width / height, 1, 100);
    glMatrixMode(GL_MODELVIEW);
}

// Initialize GLUT and start main loop
int main(int argc, char** argv) {
    balls[0].position = {5.0, 6.0, 7.0};
    balls[0].velocity = {4.0, 6.0, 7.0};
    oldTime = timeGetTime();
    glutInit(&argc, argv);
    glutInitWindowSize(window_width, window_height);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutCreateWindow("Falling Ball");
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    GL_Setup(window_width, window_height);
    glutMainLoop();
}