#include <GLUT/glut.h>
#include <sys/time.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <math.h>
#define window_width  1000
#define window_height 1000

typedef struct {
    double x;
    double y;
    double z;
}TVector;

typedef struct {
    TVector oldposition;
    TVector position;
    TVector oldvelocity;
    TVector tempvelocity;
    TVector velocity;
    TVector equilib;
    bool    spring;
    int     relobj;
    double  collidetime;
}TObject3D;

// angle of rotation for camera
float       angle = 0.0f;
// actual vector representing the camera's direction
float       lx = 0.0f, lz = -1.0f;
// XZ position of the camera
float       camx = 0.0f, camz = 75.0f;

// the key states where variables will be zero when no key is being presses
float       deltaAngle = 0.0f;
float       deltaForwardMove = 0;
float       deltaSideMove = 0;
int         xOrigin = -1;

// physics constants
int         v = 5;
int         k = 1;
float       g = 0.5;
int         radius = 1;
int         dt = 25;

// room variables
int         roomXmax = window_width/5;
int         roomXmin = -window_width/5;
int         posmax = window_width/20 - 20;
int         posmin = -window_width/20 + 20;
int         velmax = 10;
int         velmin = -10;
char        *theProgramTitle;
bool        isAnimating = true;
GLuint      currentTime;
GLuint      oldTime;
int         numballs = 200;
TObject3D   *balls = new TObject3D[numballs];


// render delay 25 milliseconds
const GLuint ANIMATION_DELAY = 25;

// Main loop
GLuint timeGetTime() {
    timeval time;
    gettimeofday(&time, NULL);
    return GLuint(time.tv_sec * 1000 + time.tv_usec / 1000);
}

void initialize() {
    for (int i = 0; i < numballs; i++) {
        balls[i].position = {
            static_cast<double>(rand() % (posmax - posmin) + posmin),
            static_cast<double>(rand() % (posmax - posmin) + posmin),
            static_cast<double>(rand() % (posmax - posmin) + posmin)
        };
        balls[i].velocity = {
            static_cast<double>(rand() % (velmax - velmin) + velmin),
            static_cast<double>(rand() % (velmax - velmin) + velmin),
            static_cast<double>(rand() % (velmax - velmin) + velmin)
        };
        balls[i].spring = false;
    }
    oldTime = timeGetTime();
}

void computePos(float deltaforward, float deltaside){
    camx += deltaforward * lx * 2;
    camz += deltaforward * lz * 2;
    camx += deltaside * (-lz) * 2;
    camz += deltaside * lx * 2;
    
    if (camx >= roomXmax) {
        camx = roomXmax - 1;
    }
    if (camx <= roomXmin) {
        camx = roomXmin + 1;
    }
    if (camz >= 180) {
        camz = 180 - 1;
    }
    if (camz <= -180) {
        camz = -180 + 1;
    }
}

void computeDir(float deltaAngle){
    angle += deltaAngle * .25;
    lx = sin(angle);
    lz = -cos(angle);
}

void move(double dt) {
    for (int i = 0; i < numballs; i++) {
        double newdt = dt / 100;
        
        // store old positions
        balls[i].oldposition = {
            balls[i].position.x,
            balls[i].position.y,
            balls[i].position.z
        };
        
        // update positions
        balls[i].position = {
            balls[i].oldposition.x + balls[i].velocity.x * newdt,
            balls[i].oldposition.y + balls[i].velocity.y * newdt,
            balls[i].oldposition.z + balls[i].velocity.z * newdt
        };
    }
}

void borderRepos(int i){
    
    // Reposition everything if on the borders
    if (balls[i].position.x <= roomXmin){
        balls[i].position.x = roomXmin + 1;
    }
    if (balls[i].position.x >= roomXmax){
        balls[i].position.x = roomXmax - 1;
    }
    if (balls[i].position.y <= -window_height/20 + 15){
        balls[i].position.y = -window_height/20 + 15 + 1;
    }
    if (balls[i].position.y >= window_height/20 - 15){
        balls[i].position.y = window_height/20 - 15 - 1;
    }
    if (balls[i].position.z <= -180){
        balls[i].position.z = -180 + 1;
    }
    if (balls[i].position.z >= 180){
        balls[i].position.z = 180 - 1;
    }
}

void sticky(double dt) {
    double newdt = dt / 100;
    
    for (int i = 0; i < numballs; i++) {
        
        // get the displacement from equilibrium
        double dx = balls[i].oldposition.x - balls[i].equilib.x;
        double dy = balls[i].oldposition.y - balls[i].equilib.y;
        double dz = balls[i].oldposition.z - balls[i].equilib.z;
        
        // if the the balls have to act as a spring
        if (balls[i].spring == true) {
            
            // store old velocities
            balls[i].oldvelocity = {
                balls[i].velocity.x,
                balls[i].velocity.y,
                balls[i].velocity.z
            };
            
            // update velocities using spring force
            balls[i].velocity = {
                balls[i].oldvelocity.x - newdt * k * dx,
                balls[i].oldvelocity.y - newdt * k * dy,
                balls[i].oldvelocity.z - newdt * k * dz
            };
            
            // if the balls have gone over a certain sticky force field
            if (dx > v || dy > v || dz > v) {
                balls[i].spring = false;
            }
        }
        
        // reposition balls if they disappear
        if (isnan(balls[i].velocity.x) || isnan(balls[i].velocity.y) || isnan(balls[i].velocity.z)) {
            balls[i].position = {
                (camx + 360 * lx)/2 + static_cast<double>(rand() % (velmax - velmin) + velmin),
                1 + static_cast<double>(rand() % (velmax - velmin) + velmin),
                (camz + 360 * lz)/2 + static_cast<double>(rand() % (velmax - velmin) + velmin)
            };
            
            // reposition balls if they're past the border
            borderRepos(i);
            
            balls[i].velocity = {
                static_cast<double>(rand() % (velmax - velmin) + velmin),
                static_cast<double>(rand() % (velmax - velmin) + velmin),
                static_cast<double>(rand() % (velmax - velmin) + velmin)
            };
        }
    }
}

void gravity(double dt) {
    for (int i = 0; i < numballs; i++) {
        double newdt = dt / 100;
        
        // store old y velocity
        balls[i].oldvelocity.y = balls[i].velocity.y;
        
        // update y velocity using gravity
        balls[i].velocity.y = balls[i].oldvelocity.y - g * newdt;
    }
}

void edge() {
    for (int i = 0; i < numballs; i++) {
        
        // Reverse velocities if at edge
        if (balls[i].position.x <= roomXmin || balls[i].position.x >= roomXmax) {
            balls[i].velocity.x = -balls[i].velocity.x;
        }
        if (balls[i].position.y <= -window_height/20 + 15 || balls[i].position.y >= window_height/20 - 15) {
            balls[i].velocity.y = -balls[i].oldvelocity.y;
        }
        if (balls[i].position.z <= -180 || balls[i].position.z >= 180) {
            balls[i].velocity.z = -balls[i].velocity.z;
        }
        
        // Reposition balls if they're past border
        borderRepos(i);
    }
}

double dotProduct(TVector a, TVector b){
    double dot = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    return dot;
    
}

bool collisionTest(int i, int j) {
    // difference position vectors
    TVector * deltapos = new TVector;
    *deltapos = {
        balls[i].position.x - balls[j].position.x,
        balls[i].position.y - balls[j].position.y,
        balls[i].position.z - balls[j].position.z
    };
    
    // difference velocity vectors
    TVector * deltavel = new TVector;
    *deltavel = {
        balls[i].velocity.x - balls[j].velocity.x,
        balls[i].velocity.y - balls[j].velocity.y,
        balls[i].velocity.z - balls[j].velocity.z
    };
    
    // double the radius, then square it
    double r = radius * 2;
    r *= r;
    
    // dot product of position vectors, if it is negative they already overlap
    double c = dotProduct(*deltapos, *deltapos) - r;
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
    
    // figure out the time when the ball collided
    balls[i].collidetime = (-b - sqrt(d)) / a;
    balls[j].collidetime = (-b - sqrt(d)) / a;
    
    return (d > 0);
}

void collide() {
    for (int i = 0; i < numballs; i++) {
        for (int j = i + 1; j < numballs; j++) {
            if (collisionTest(i, j)) {
                // move the two balls to their intersecting point
                balls[i].position = {
                    balls[i].oldposition.x + balls[i].velocity.x * balls[i].collidetime,
                    balls[i].oldposition.y + balls[i].velocity.y * balls[i].collidetime,
                    balls[i].oldposition.z + balls[i].velocity.z * balls[i].collidetime
                };
                
                balls[j].position = {
                    balls[j].oldposition.x + balls[j].velocity.x * balls[j].collidetime,
                    balls[j].oldposition.y + balls[j].velocity.y * balls[j].collidetime,
                    balls[j].oldposition.z + balls[j].velocity.z * balls[j].collidetime
                };
                
                // set the two balls' equlibrium point to their intersecting point
                balls[i].equilib = {balls[i].position.x, balls[i].position.y, balls[i].position.z};
                
                balls[j].equilib = {balls[j].position.x, balls[j].position.y, balls[j].position.z};
                
                // store old velocities of x and z because they've never been stored before
                balls[i].oldvelocity = {balls[i].velocity.x, balls[i].oldvelocity.y, balls[i].velocity.z};
                balls[j].oldvelocity = {balls[j].velocity.x, balls[j].oldvelocity.y, balls[j].velocity.z};
                
                // update the new velocities through switching them
                balls[i].velocity = {balls[j].oldvelocity.x, balls[j].oldvelocity.y, balls[j].oldvelocity.z};
                
                balls[j].velocity = {balls[i].oldvelocity.x, balls[i].oldvelocity.y, balls[i].oldvelocity.z};
                
                // now turn on the sticky force for both objects
                balls[i].spring = true;
                balls[j].spring = true;
                
                // identify the other ball that it's sticky to
                balls[i].relobj = j;
                balls[j].relobj = i;
            }
        }
    }
}

void bend(float deltaside) {
    for (int i = 0; i < numballs; i++) {
        if (balls[i].position.x < (lx - camx) + 1 || balls[i].position.z >  (lz - camz)  - 1) {
            balls[i].velocity = {0,0,0};
        }
        
        balls[i].oldvelocity = balls[i].velocity;
        
        // Displacement between the box and the balls
        float dx = (camx + 360 * lx)/2  - balls[i].oldposition.x;
        float dy = 1.0f - balls[i].oldposition.y;
        float dz = (camz + 360 * lz)/2 - balls[i].oldposition.z;
        
        // Change the direction of the balls to be attracted to a point
        balls[i].velocity = {dx/dt, dy/dt, dz/dt};
    };
}

void makelighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    GLfloat light_ambient[] = {0.1, 0.1, 0.1, 1.0};
    GLfloat light_position[] = {-1.0, 1.0, -1.0, 0.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
}

void makewalls() {
	glBegin(GL_QUADS);
    
    // front wall
    glColor3f(0.9, 0.9, 0.9);
    glVertex3f(roomXmin, -window_height/20 + 15, -180.0f);
    glColor3f(0.9, 0.9, 0.9);
    glVertex3f(roomXmax, -window_height/20 + 15,  -180.0f);
    glColor3f(0.5, 0.5, 0.5);
    glVertex3f(roomXmax, window_height/20 + 15,  -180.0f);
    glColor3f(0.5, 0.5, 0.5);
    glVertex3f(roomXmin, window_height/20 + 15, -180.0f);
	glEnd();
    
    // side wall
    glBegin(GL_QUADS);
    glVertex3f(roomXmax, -window_height/20 + 15, -180.0f);
    glVertex3f(roomXmax, -window_height/20 + 15,  180.0f);
    glVertex3f(roomXmax, window_height/20 + 15,  180.0f);
    glVertex3f(roomXmax, window_height/20 + 15, -180.0f);
	glEnd();
    
    // side wall
    glBegin(GL_QUADS);
    glVertex3f(roomXmin, -window_height/20 + 15, -180.0f);
    glVertex3f(roomXmin, -window_height/20 + 15,  180.0f);
    glVertex3f(roomXmin, window_height/20 + 15,  180.0f);
    glVertex3f(roomXmin, window_height/20 + 15, -180.0f);
	glEnd();
    
    // back wall
    glBegin(GL_QUADS);
    glColor3f(0.9, 0.9, 0.9);
    glVertex3f(roomXmin, -window_height/20 + 15, 180.0f);
    glColor3f(0.9, 0.9, 0.9);
    glVertex3f(roomXmax, -window_height/20 + 15,  180.0f);
    glColor3f(0.5, 0.5, 0.5);
    glVertex3f(roomXmax, window_height/20 + 15,  180.0f);
    glColor3f(0.5, 0.5, 0.5);
    glVertex3f(roomXmin, window_height/20 + 15, 180.0f);
	glEnd();
    
    // floor
    glBegin(GL_QUADS);
    glColor3f(0.2, 0.2, 0.2);
    glVertex3f(roomXmin, -window_height/20 + 15, -180.0f);
    glVertex3f(roomXmin, -window_height/20 + 15,  180.0f);
    glVertex3f(roomXmax, -window_height/20 + 15,  180.0f);
    glVertex3f(roomXmax, -window_height/20 + 15, -180.0f);
    glEnd();
}

void makeball(TObject3D ball) {
    // Draw a sphere
    glColor3f(ball.velocity.x/5, ball.velocity.y/5, ball.velocity.z/5);
    glPushMatrix();
    glTranslatef(ball.position.x, ball.position.y, ball.position.z);
    glutSolidSphere(radius, 30, 30);
    glPopMatrix();
}

void makecubes() {
    // Draw 2 cubes
    glLoadIdentity();
    glPushMatrix();
    glTranslatef(-7, -5, -20);
    glColor3f(0.9, 0.9, 0.9);
    glutSolidCube(2);
    glPopMatrix();
    
    glLoadIdentity();
    glPushMatrix();
    glTranslatef(7, -5, -20);
    glColor3f(0.9, 0.9, 0.9);
    glutSolidCube(2);
    glPopMatrix();
}

void display() {
    
    computePos(deltaForwardMove, deltaSideMove);
    
    if(deltaAngle){
        computeDir(deltaAngle);
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Load identity matrix
    glLoadIdentity();
    
    gluLookAt(camx,     1.0f,   camz,
              camx+lx,  1.0f,   camz+lz,
              0.0f,     1.0f,   0.0f);
    
    
    makelighting();
    makewalls();
    for (int i = 0; i < numballs; i++) {
        makeball(balls[i]);
    }
    makecubes();
    
    // Swap buffers (color buffers, makes previous render visible)
    glutSwapBuffers();
}

void processNormalKeys(unsigned char key, int x, int y) {
    switch (key) {
        case 'g':
            g = 0;
            break;
        case ' ':
            for (int i = 0; i < numballs; i++) {
                balls[i].tempvelocity = balls[i].oldvelocity;
            }
            bend(deltaAngle);
            break;
    }
}

void processSpecialKeys(int key, int xx, int yy) {
    
	switch (key) {
		case GLUT_KEY_LEFT :
            deltaSideMove = -0.45;
			break;
		case GLUT_KEY_RIGHT :
            deltaSideMove = 0.45;
			break;
		case GLUT_KEY_UP :
            deltaForwardMove = 0.45;
			break;
		case GLUT_KEY_DOWN :
            deltaForwardMove = -0.45;
			break;
	}
}

void releaseKey (unsigned char key, int x, int y) {
    switch (key) {
        case 'g':
            g = 0.5;
            break;
        case ' ':
            for (int i = 0; i < numballs; i++) {
                balls[i].oldvelocity = balls[i].tempvelocity;
            }
            break;
        default:
            break;
    }
}

void releaseSpecialKey (int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT :
		case GLUT_KEY_RIGHT :
            deltaSideMove = 0;
			break;
		case GLUT_KEY_UP :
		case GLUT_KEY_DOWN :
            deltaForwardMove = 0;
			break;
    }
}

void mouseMove(int x, int y){
    // this will only be true when the left button is down
    if (xOrigin >= 0) {
        
		// update deltaAngle
		deltaAngle = (x - xOrigin) * 0.001f;
	}
}

void mouseButton(int button, int state, int x, int y) {
    
	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {
        
		// when the button is released
		if (state == GLUT_UP) {
			angle += deltaAngle;
			xOrigin = -1;
            deltaAngle = 0;
		}
		else  {
			xOrigin = x;
		}
	}
}

void idle () {
    if (isAnimating) {
        currentTime = timeGetTime();
        if ((currentTime - oldTime) > ANIMATION_DELAY) {
            
            // move the balls
            edge();
            move(dt);
            collide();
            gravity(dt);
            sticky(dt);
            
            // compute the frame rate
            oldTime = currentTime;
        }
        
        // notify window it has to be repainted
        glutPostRedisplay();
    }
}

// Initialze OpenGL perspective matrix
void GL_Setup(int width, int height) {
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
    glutKeyboardUpFunc(releaseKey);
	glutSpecialFunc(processSpecialKeys);
    glutSpecialUpFunc(releaseSpecialKey);
    
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMove);
    
    glutMainLoop();
}