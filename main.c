#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h>

unsigned char bitmap[8]; // 8x8 boolean grid

void isometric() {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  double d = 10 / sqrt(3);
  gluLookAt(d, d, d, // camera position, 10 away from origin
	    0, 0, 0, // origin is at center
	    0, 0, 1  // z-axis is upwards
	    );

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-10, 10,
	  -10, 10,
	  1, 20);
}

// where does the line through p and q interecept the plane z=0?
void intercept(double *p, double *q, double *x, double *y) {
  double t = p[2] / (p[2] - q[2]); // (1-t)*p + t*q = 0 <=> p - t*p + t*q = 0 <=> t = p/(p-q)
  *x = (1 - t) * p[0] + t * q[0];
  *y = (1 - t) * p[1] + t * q[1];
}

void init() {
  srand(time(NULL));

  int x, y;
  for (y = 0; y < 8; y++)
    bitmap[y] = (unsigned char)rand();

  glEnable(GL_DEPTH_TEST);

  float light[] = {0.1, 0.2, 0.3, 0};
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, light);

  isometric();
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  int x, y;
  glTranslated(-3.5, -3.5, 0);
  for (y = 0; y < 8; y++) {
    glPushMatrix();
    for (x = 0; x < 8; x++) {
      if (bitmap[y] & (1 << x))
	glutSolidCube(1);
      glTranslated(1, 0, 0);
    }
    glPopMatrix();
    glTranslated(0, 1, 0);
  }

  glPopMatrix();
  glutSwapBuffers();
}

void mouse(int button, int state, int u, int v) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
    double model[16], projection[16], x, y, z, near[3], far[3];
    int viewport[4];
    float depth;

    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    v = viewport[3] - v;

    glReadPixels(u, v, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

    if (depth < 1) {
      gluUnProject(u, v, depth,
		   model, projection, viewport,
		   &x, &y, &z);
      printf("%f %f %f\n", x, y, z);
    }
    else {
      gluUnProject(u, v, 0,
		   model, projection, viewport,
		   near + 0, near + 1, near + 2);
      gluUnProject(u, v, depth,
		   model, projection, viewport,
		   far + 0, far + 1, far + 2);
      intercept(near, far, &x, &y);
      z = 0;
      printf("%f %f\n", x, y);
    }

    x += 4;
    y += 4;
    if (z < 1 && x >= 0 && x < 8 && y >= 0 && y < 8)
      bitmap[(int)y] ^= 1 << (int)x;

    glutPostRedisplay();
  }
}

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(640, 640);
  glutCreateWindow("iso");

  init();
  glutDisplayFunc(display);
  glutMouseFunc(mouse);

  glutMainLoop();
  return 0;
}
