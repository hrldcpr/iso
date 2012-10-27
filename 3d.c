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

const int VIEW_WIDTH = 400, VIEW_HEIGHT = 400;
const int WIDTH = 8, HEIGHT = 8, DEPTH = 8;
const float RADIUS = 0.2;
const float VELOCITY = 0.5; // per second

Cube *cubes = NULL; // linked list of cubes, for easy insertion / removal
Ball *balls = NULL;
int mouse_x, mouse_y;
char dragging = 0;
long prev_time = -1;
float theta = 0;

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

void orthographic() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-10, 10,
	  -10, 10,
	  1, 20);
}

void lookFrom(char cx, char cy, char cz,
              char upx, char upy, char upz) {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  float d = 10 / sqrt(cx + cy + cz);
  gluLookAt(d * cx, d * cy, d * cz, // camera position, 10 away from origin
	    0, 0, 0, // origin is at center
	    upx, upy, upz  // z-axis is upwards
	    );
  glRotatef(theta, 0, 0, 1);
}

// where does the line through p and q interecept the plane z=0?
void intercept(double *p, double *q, double *x, double *y) {
  double t = p[2] / (p[2] - q[2]); // (1-t)*p + t*q = 0 <=> p - t*p + t*q = 0 <=> t = p/(p-q)
  *x = (1 - t) * p[0] + t * q[0];
  *y = (1 - t) * p[1] + t * q[1];
}

void rainbow(double x, unsigned char *r, unsigned char *g, unsigned char *b) {
  x = fmodf(x, 1); // cycle through [0, 1)
  x *= 6; // we will walk six edges of the RGB cube, so it's easiest to go from 0 to 6.

  if (x < 1) { // red to yellow
    *r = 255; *b = 0; *g = 256 * x;
  } else if (x < 2) { // yellow to green
    *b = 0; *g = 255; *r = 255 * (2 - x);
  } else if (x < 3) { // green to turquoise
    *g = 255; *r = 0; *b = 256 * (x - 2);
  } else if (x < 4) { // turquoise to blue
    *r = 0; *b = 255; *g = 255 * (4 - x);
  } else if (x < 5) { // blue to purple
    *b = 255; *g = 0; *r = 256 * (x - 4);
  } else { // purple to red
    *g = 0; *r = 255; *b = 255 * (6 - x);
  }
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
  for (i = 6; i >= 1; i--) {
    add_cube(i - WIDTH/2, i - HEIGHT/2, i + 1);
    rainbow(i / 6.0, &r, &g, &b);
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

  orthographic();

  staircase();
}

void draw() {
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glBlendFunc(GL_ONE, GL_ZERO); // the default blend func, which ignores alpha
  glColor4f(1, 1, 1, 0); // zero alpha, so that the balls we draw later using alpha-blending interact with each other but not with the cubes
  Cube *cube = cubes;
  while (cube) {
    glPushMatrix();
    glTranslatef(cube->x, cube->y, cube->z);
    glutSolidCube(1);
    glPopMatrix();
    cube = cube->next;
  }

  // TODO disable depth buffer before rendering balls? not so simple, since we still want depth test.
  glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA); // additive blending, so colors can combine to white

  glPushMatrix();
  glTranslatef(0, 0, 1.5);
  glColor4f(1, 0, 0, 1);
  glutSolidCube(1);
  glTranslatef(1, 1, 1);
  glColor4f(0, 1, 0, 1);
  glutSolidCube(1);
  glTranslatef(1, 1, 1);
  glColor4f(0, 0, 1, 1);
  glutSolidCube(1);
  glPopMatrix();

  glTranslatef(0, 0, 0.5 + RADIUS); // spheres sit on top of cubes
  Ball *ball = balls;
  while (ball) {
    glPushMatrix();
    glTranslatef(ball->x, ball->y, ball->z);
    glColor4ub(ball->r, ball->g, ball->b, 255);
    glutSolidSphere(RADIUS, 10, 10);
    glPopMatrix();
    ball = ball->next;
  }

  glPopMatrix();
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, VIEW_HEIGHT, VIEW_WIDTH, VIEW_HEIGHT);
  lookFrom(1, 1, 1, 0, 0, 1); // isometric
  draw();

  glViewport(VIEW_WIDTH, VIEW_HEIGHT, VIEW_WIDTH, VIEW_HEIGHT);
  lookFrom(1, 0, 0, 0, 0, 1); // YZ
  draw();

  glViewport(0, 0, VIEW_WIDTH, VIEW_HEIGHT);
  lookFrom(0, 1, 0, 0, 0, 1); // XZ
  draw();

  glViewport(VIEW_WIDTH, 0, VIEW_WIDTH, VIEW_HEIGHT);
  lookFrom(0, 0, 1, 0, 1, 0); // YZ
  draw();

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

        glViewport(0, VIEW_HEIGHT, VIEW_WIDTH, VIEW_HEIGHT);
        lookFrom(1, 1, 1, 0, 0, 1); // isometric

	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	v = 2 * VIEW_HEIGHT - v;

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

    theta += u - mouse_x;
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
	/* if (x != round(ball->x)) */
	/*   ball->x += RADIUS; */
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
  glutInitWindowSize(2 * VIEW_WIDTH, 2 * VIEW_HEIGHT);
  glutCreateWindow("iso");

  init();
  glutDisplayFunc(display);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutIdleFunc(idle);

  glutMainLoop();
  return 0;
}
