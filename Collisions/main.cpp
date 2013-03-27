
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

double      radius = 1;
double      dt = 50;
int         posmax = 20;
int         posmin = -20;
int         velmax = 5;
int         velmin = -5;
char        *theProgramTitle;
bool        isAnimating = true;
GLuint      currentTime;
GLuint      oldTime;
int         numballs = 2;
TObject3D   *balls = new TObject3D[numballs];


// render delay 100 milliseconds
const GLuint ANIMATION_DELAY = 40;

// Main loop
GLuint timeGetTime() {
    timeval time;
    gettimeofday(&time, NULL);
    return GLuint(time.tv_sec * 1000 + time.tv_usec / 1000);
}

void initialize() {
//    for (int i = 0; i < numballs; i++) {
//        balls[i].position = {static_cast<double>(rand() % (posmax - posmin) + posmin), static_cast<double>(rand() % (posmax - posmin) + posmin), static_cast<double>(rand() % (posmax - posmin) + posmin)};
//        balls[i].velocity = {static_cast<double>(rand() % (velmax - velmin) + velmin), static_cast<double>(rand() % (velmax - velmin) + velmin), static_cast<double>(rand() % (velmax - velmin) + velmin)};
//    }
    balls[0].position = {0.0, -30.0, 0.0};
    balls[0].velocity = {0.0, -5.0, 0.0};
    balls[1].position = {0.0, 10.0, 0.0};
    balls[1].velocity = {0.0, 1.0, 0.0};
    oldTime = timeGetTime();
}

void fall(double dt) {
    for (int i = 0; i < numballs; i++) {
        double newdt = dt / 100;
        
        // store old velocity
        balls[i].oldvelocity.y = balls[i].velocity.y;
        
        // store old positions
        balls[i].oldposition.x = balls[i].position.x;
        balls[i].oldposition.y = balls[i].position.y;
        balls[i].oldposition.z = balls[i].position.z;
        
        // update positions
        balls[i].position.x = balls[i].position.x + balls[i].velocity.x * newdt;
        balls[i].position.y = balls[i].position.y + balls[i].velocity.y * newdt;
        balls[i].position.z = balls[i].position.z + balls[i].velocity.z * newdt;
        
        // update velocity
        balls[i].velocity.y = balls[i].velocity.y - gravity * newdt;
    }
}

void bounce() {
    for (int i = 0; i < numballs; i++) {
        if (balls[i].position.y <= -window_height/20 + 15 || balls[i].position.y >= window_height/20 - 15) {
            balls[i].velocity.y = -balls[i].oldvelocity.y;
        }
        if (balls[i].position.x <= -window_width/20 + 15 || balls[i].position.x >= window_width/20 - 15) {
            balls[i].velocity.x = -balls[i].velocity.x;
        }
        if (balls[i].position.z <= -50 || balls[i].position.z >= 30) {
            balls[i].velocity.z = -balls[i].velocity.z;
        }
    }
}

double dotProduct(TVector a, TVector b){
    double dot = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    return dot;
    
}

TVector crossProduct(TVector a, TVector b) {
    TVector *cross = new TVector;
    cross->x = (a.y * b.z) - (a.z * b.y);
    cross->y = (a.z * b.x) - (a.x * b.z);
    cross->z = (a.x * b.y) - (a.y * b.x);
    return *cross;
}

bool collisionTest(int i, int j) {
    // difference position vectors
    TVector * deltapos = new TVector;
    deltapos->x = balls[i].position.x - balls[j].position.x;
    deltapos->y = balls[i].position.y - balls[j].position.y;
    deltapos->z = balls[i].position.z - balls[j].position.z;
    
    // difference velocity vectors
    TVector * deltavel = new TVector;
    deltavel->x = balls[i].velocity.x - balls[j].velocity.x;
    deltavel->y = balls[i].velocity.y - balls[j].velocity.y;
    deltavel->z = balls[i].velocity.z - balls[j].velocity.z;
    
    // double the radius, then square it
    double r = radius * 2;
    r *= r;
    
    // dot product of position vectors, if it is negative they already overlap
    double c = dotProduct(*deltapos, *deltapos) - r / 5;
    if (c < 0) {
        return true;
    }
    
    // dot product of velocity vectors
    double a = dotProduct(*deltavel, *deltavel);
    
    // dot product of velocity vector by position vector, if it is 0 or positive they are not moving towards each other
    double b = dotProduct(*deltavel, *deltapos);
    if (b >= 0) {
        return false;
    }
    
    // through Viete's theorem, check if spheres can intersect within 1 frame
    if ((a + b) <= 0 && (a + 2 * b + c) >= 0) {
        return false;
    }
    
    // if d is negative, there are no real roots and no collisions
    double d = b * b - a * c;
    return (d > 0);
}

void collision() {
    for (int i = 0; i < numballs; i++) {
        for (int j = i + 1; j < numballs; j++) {
            if (collisionTest(i, j)) {
                balls[i].velocity.x = balls[j].oldvelocity.x;
                balls[j].velocity.x = balls[i].oldvelocity.x;
                balls[i].velocity.y = balls[j].oldvelocity.y;
                balls[j].velocity.y = balls[i].oldvelocity.y;
                balls[i].velocity.z = balls[j].oldvelocity.z;
                balls[j].velocity.z = balls[i].oldvelocity.z;
                std::cout << "I'M HIT" << std::endl;
            }
        }
    }
}

void makeball(TObject3D ball) {
    // Draw a sphere
    glPushMatrix();
    glTranslatef(ball.position.x, ball.position.y, ball.position.z);
    glutSolidSphere(radius, 30, 30);
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
            fall(dt);
            bounce();
            collision();
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
