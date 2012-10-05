#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h>

const int W = 8, H = 8, Z = 8;
unsigned char *map;

int mouseX, mouseY;
char dragging = 0;


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

  map = malloc(W * H);
  int i;
  for (i = 0; i < W * H; i++)
    map[i] = rand() % 2; //(Z / 2);

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
  glTranslated(-0.5 * (W - 1), -0.5 * (H - 1), 0);
  for (y = 0; y < H; y++) {
    for (x = 0; x < W; x++) {
      if (map[y*W + x]) {
	glPushMatrix();
	glTranslated(x, y, map[y*W + x]);
	glutSolidCube(1);
	glPopMatrix();
      }
    }
  }

  // staircase:
  int i;
  for (i = 0; i < 6; i++) {
    glTranslated(1, 1, 1);
    glutSolidCube(1);
    /* glPushMatrix(); */
    /* glTranslated(0, 0, 0.7); */
    /* glutSolidCube(0.2); */
    /* glPopMatrix(); */
  }

  glPopMatrix();
  glutSwapBuffers();
}

void mouse(int button, int state, int u, int v) {
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      mouseX = u;
      mouseY = v;
    }
    else if (state == GLUT_UP) {
      if (!dragging) {
	double model[16], projection[16], x, y, z, near[3], far[3];
	int viewport[4];
	float depth;

	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	v = viewport[3] - v;

	glReadPixels(u, v, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

	if (depth > 0 && depth < 1) { // not clipped
	  gluUnProject(u, v, depth,
		       model, projection, viewport,
		       &x, &y, &z);
	}
	else { // clipped or empty
	  gluUnProject(u, v, 0,
		       model, projection, viewport,
		       near + 0, near + 1, near + 2);
	  gluUnProject(u, v, depth,
		       model, projection, viewport,
		       far + 0, far + 1, far + 2);
	  intercept(near, far, &x, &y);
	  z = 0;
	}

	x += 0.5 * W;
	y += 0.5 * H;
	if (x >= 0 && x < W && y >= 0 && y < H)
	  map[(int)y*W + (int)x] ^= 1;
      }

      dragging = 0;
    }

    glutPostRedisplay();
  }
}

void motion(int u, int v) {
  if (u != mouseX || v != mouseY) {
    dragging = 1;

    glMatrixMode(GL_MODELVIEW);
    glRotatef(u - mouseX, 0, 0, 1);
    glutPostRedisplay();

    mouseX = u;
    mouseY = v;
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
  glutMotionFunc(motion);

  glutMainLoop();
  return 0;
}