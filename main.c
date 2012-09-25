#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h>

const double s = 0.1;

void isometric() {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  double d = sqrt(1.0 / 3);
  gluLookAt(d, d, d, // camera position, 1 away from origin
	    0, 0, 0, // origin is at center
	    0, 0, 1  // z-axis is upwards
	    );

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1, 1,
	  -1, 1,
	  s, 1000
	  );
}

void init() {
  srand(time(NULL));

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

  int n = 5, x, y;
  glTranslated(-n * s, -n * s, 0);
  for (y = -n; y <= n; y++) {
    glPushMatrix();
    for (x = -n; x <= n; x++) {
      if (rand() % 2)
	glutSolidCube(s);
      glTranslated(s, 0, 0);
    }
    glPopMatrix();
    glTranslated(0, s, 0);
  }

  glPopMatrix();
  glutSwapBuffers();
}

void mouse(int button, int state, int u, int v) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
    double model[16], projection[16], near[3], far[3];
    int viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluUnProject(u, v, 0,
		 model, projection, viewport,
		 near + 0, near + 1, near + 2);
    gluUnProject(u, v, 1,
		 model, projection, viewport,
		 far + 0, far + 1, far + 2);

    printf("%f %f %f\n", near[0], near[1], near[2]);
    printf("%f %f %f\n", far[0], far[1], far[2]);
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
