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

typedef struct Ball {
  float x;
  float y;
  float z;
  unsigned char r;
  unsigned char g;
  unsigned char b;
  Cube *cube;
  struct Ball *next;
} Ball;

const int WIDTH = 8, HEIGHT = 8, DEPTH = 8;
const float RADIUS = 0.2;
const float VELOCITY = 0.1; // per second

Cube *cubes = NULL; // linked list of cubes, for easy insertion / removal
Ball *balls = NULL;
int mouse_x, mouse_y;
char dragging = 0;
long prev_time = -1;

void add_cube(int x, int y, int z) {
  Cube *cube = malloc(sizeof(Cube));
  cube->x = x;
  cube->y = y;
  cube->z = z;
  cube->next = cubes;
  cubes = cube;
}

void add_ball(float x, float y, float z,
	      unsigned char r, unsigned char g, unsigned char b,
	      Cube *cube) {
  Ball *ball = malloc(sizeof(Ball));
  ball->x = x;
  ball->y = y;
  ball->z = z;
  ball->r = r;
  ball->g = g;
  ball->b = b;
  ball->cube = cube;
  ball->next = balls;
  balls = ball;
}

void add_ball_at(float x, float y, float z,
		 unsigned char r, unsigned char g, unsigned char b) {
  add_ball(x, y, z, r, g, b, NULL);
}

void add_ball_on(unsigned char r, unsigned char g, unsigned char b,
		 Cube *cube) {
  add_ball(cube->x, cube->y, cube->z, r, g, b, cube);
}

void isometric() {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  float d = 10 / sqrt(3);
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

void staircase() {
  int x, y, i;
  // make a grid
  for (y = -HEIGHT/2; y < HEIGHT/2; y++) {
    for (x = -WIDTH/2; x < WIDTH/2; x++)
      add_cube(x, y, 0);
  }

  // make an ambiguous staircase
  unsigned char r, g, b;
  for (i = 7; i >= 0; i--) {
    add_cube(i - WIDTH/2, i - HEIGHT/2, i + 1);
    if (i <= 3) r = 255 >> (3 - i);
    if (i <= 5) g = 255 >> (5 - i);
    b = 255 >> (7 - i);
    add_ball_on(r, g, b, cubes);
  }
}

void init() {
  srand(time(NULL));

  glEnable(GL_DEPTH_TEST);

  float light[] = {0.1, 0.2, 0.3, 0};
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, light);
  glEnable(GL_COLOR_MATERIAL); // makes glColor work with GL_LIGHTING enabled

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  isometric();

  staircase();
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glColor4f(1, 1, 1, 1);
  Cube *cube = cubes;
  while (cube) {
    glPushMatrix();
    glTranslatef(cube->x, cube->y, cube->z);
    glutSolidCube(1);
    glPopMatrix();
    cube = cube->next;
  }

  // TODO disable depth buffer before rendering balls? not so simple, since we still want depth test.
  // TODO use glColor4 and glBlendFunc to give alpha-blending to balls. need to draw from back-to-front lest only the first one passes depth test.
  glTranslatef(0, 0, 0.5 + RADIUS); // spheres sit on top of cubes
  Ball *ball = balls;
  while (ball) {
    glPushMatrix();
    glTranslatef(ball->x, ball->y, ball->z);
    glColor4ub(ball->r, ball->g, ball->b, 128);
    glutSolidSphere(RADIUS, 10, 10);
    glPopMatrix();
    ball = ball->next;
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
	while (next) {
	  if (next->x == x && next->y == y && next->z == z)
	    break;
	  prev = &next->next;
	  next = *prev;
	}
	if (next) {// hit
	  *prev = next->next; // remove from list
	  Ball *ball = balls;
	  while(ball) {
	    if (ball->cube == next)
	      ball->cube = NULL;
	    ball = ball->next;
	  }
	}
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
    //glRotatef(v - mouse_y, 1, 0, 0);
    glutPostRedisplay();

    mouse_x = u;
    mouse_y = v;
  }
}

void idle() {
  long t = glutGet(GLUT_ELAPSED_TIME);
  if (prev_time != -1) {
    long dt = t - prev_time;

    int x, y, z;
    Cube *cube;
    Ball *ball = balls;
    while (ball) {
      x = round(ball->x); // ball is x-centered same as cube
      y = round(ball->y); // ball is y-centered same as cube
      z = ceil(ball->z); // ball sits at the top of the cube, whereas the cube is z-centered

      if (ball->cube) {
	// balls on cubes move sideways #factoflife
	ball->x += VELOCITY * dt * 0.001;
	if (x != round(ball->x))
	  ball->x += RADIUS;
      }
      else
	// balls in space fall #factofspace
	ball->z -= 10 * VELOCITY * dt * 0.001;

      if (x != round(ball->x) || y != round(ball->y) || z != ceil(ball->z)) {
	// we moved enough to be on a new cube, so check if we are
	x = round(ball->x);
	y = round(ball->y);
	z = ceil(ball->z);
	cube = cubes;
	while (cube) {
	  if (x == cube->x && y == cube->y && z == cube->z)
	    break;
	  cube = cube->next;
	}
	ball->cube = cube;
      }

      ball = ball->next;
    }

    glutPostRedisplay();
  }
  prev_time = t;
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
  glutIdleFunc(idle);

  glutMainLoop();
  return 0;
}
