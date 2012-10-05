#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h>

typedef struct Cube {
  int x;
  int y;
  int z;
  struct Cube *next;
} Cube;

const int WIDTH = 8, HEIGHT = 8, DEPTH = 8;

Cube *cubes = NULL; // linked list of cubes, for easy insertion / removal
int mouse_x, mouse_y;
char dragging = 0;


void add_cube(int x, int y, int z) {
  Cube *cube = malloc(sizeof(Cube));
  cube->x = x;
  cube->y = y;
  cube->z = z;
  cube->next = cubes;
  cubes = cube;
}

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

  int n = rand() % 32;
  while (n-- > 0)
    add_cube(rand()%WIDTH - WIDTH/2,
	     rand()%HEIGHT - HEIGHT/2,
	     rand()%DEPTH - DEPTH/2);

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

  Cube *cube = cubes;
  while (cube != NULL) {
    glPushMatrix();
    glTranslated(cube->x, cube->y, cube->z);
    glutSolidCube(1);
    glPopMatrix();
    cube = cube->next;
  }

  glPopMatrix();
  glutSwapBuffers();
}

void mouse(int button, int state, int u, int v) {
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      mouse_x = u;
      mouse_y = v;
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

	glReadPixels(u, v, 1, 1, // the 1x1 rect at (u,v)
		     GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

	if (depth > 0 && depth < 1) { // not clipped
	  gluUnProject(u, v, depth,
		       model, projection, viewport,
		       &x, &y, &z);
	}
	else { // clipped or empty
	  gluUnProject(u, v, 0,
		       model, projection, viewport,
		       near + 0, near + 1, near + 2);
	  gluUnProject(u, v, 1,
		       model, projection, viewport,
		       far + 0, far + 1, far + 2);
	  intercept(near, far, &x, &y);
	  z = 0;
	}

	x = round(x);
	y = round(y);
	z = round(z);
	Cube **prev = &cubes, *next = *prev;
	while (next != NULL
	       && !(next->x == x && next->y == y && next->z == z)) {
	  prev = &next->next;
	  next = *prev;
	}
	if (next != NULL) // hit
	  *prev = next->next; // remove from list
	else
	  add_cube(x, y, z);
      }

      dragging = 0;
    }

    glutPostRedisplay();
  }
}

void motion(int u, int v) {
  if (u != mouse_x || v != mouse_y) {
    dragging = 1;

    glMatrixMode(GL_MODELVIEW);
    glRotatef(u - mouse_x, 0, 0, 1);
    glutPostRedisplay();

    mouse_x = u;
    mouse_y = v;
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
