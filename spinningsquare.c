// gcc spinningsquare.c -o spinningsquare -lglut -lGL -lGLU -lm

#include <GL/glut.h>
#include <math.h>

float angle = 0.0f;

// Windows 95 logo colors
GLfloat red[] = {0.87f, 0.0f, 0.0f};
GLfloat green[] = {0.0f, 0.87f, 0.0f};
GLfloat blue[] = {0.0f, 0.0f, 0.87f};
GLfloat yellow[] = {0.87f, 0.87f, 0.0f};

void drawBox(float size, float gap, float depth, GLfloat color[])
{
  glColor3fv(color);

  // Front face
  glBegin(GL_QUADS);
  glVertex3f(-size - gap, size + gap, depth);
  glVertex3f(-gap, size + gap, depth);
  glVertex3f(-gap, gap, depth);
  glVertex3f(-size - gap, gap, depth);
  glEnd();

  // Back face
  glBegin(GL_QUADS);
  glVertex3f(-size - gap, size + gap, -depth);
  glVertex3f(-gap, size + gap, -depth);
  glVertex3f(-gap, gap, -depth);
  glVertex3f(-size - gap, gap, -depth);
  glEnd();

  // Top face
  glBegin(GL_QUADS);
  glVertex3f(-size - gap, size + gap, depth);
  glVertex3f(-gap, size + gap, depth);
  glVertex3f(-gap, size + gap, -depth);
  glVertex3f(-size - gap, size + gap, -depth);
  glEnd();

  // Bottom face
  glBegin(GL_QUADS);
  glVertex3f(-size - gap, gap, depth);
  glVertex3f(-gap, gap, depth);
  glVertex3f(-gap, gap, -depth);
  glVertex3f(-size - gap, gap, -depth);
  glEnd();

  // Left face
  glBegin(GL_QUADS);
  glVertex3f(-size - gap, size + gap, depth);
  glVertex3f(-size - gap, gap, depth);
  glVertex3f(-size - gap, gap, -depth);
  glVertex3f(-size - gap, size + gap, -depth);
  glEnd();

  // Right face
  glBegin(GL_QUADS);
  glVertex3f(-gap, size + gap, depth);
  glVertex3f(-gap, gap, depth);
  glVertex3f(-gap, gap, -depth);
  glVertex3f(-gap, size + gap, -depth);
  glEnd();
}

void drawWindowsLogo()
{
  float size = 0.3f;
  float gap = size * 0.1f;
  float depth = 0.05f; // Depth of the boxes

  // Draw each box with appropriate position transformations

  // Top left (red)
  glPushMatrix();
  drawBox(size, gap, depth, red);
  glPopMatrix();

  // Top right (green)
  glPushMatrix();
  glTranslatef(size + 2 * gap, 0.0f, 0.0f);
  drawBox(size, gap, depth, green);
  glPopMatrix();

  // Bottom left (blue)
  glPushMatrix();
  glTranslatef(0.0f, -(size + 2 * gap), 0.0f);
  drawBox(size, gap, depth, blue);
  glPopMatrix();

  // Bottom right (yellow)
  glPushMatrix();
  glTranslatef(size + 2 * gap, -(size + 2 * gap), 0.0f);
  drawBox(size, gap, depth, yellow);
  glPopMatrix();
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  // Move the logo back and adjust view angle
  glTranslatef(0.0f, 0.0f, -2.0f);
  glRotatef(20.0f, 1.0f, 0.0f, 0.0f); // Tilt slightly for better 3D view

  glPushMatrix();
  glRotatef(angle, 0.0f, 1.0f, 0.0f);
  drawWindowsLogo();
  glPopMatrix();

  glutSwapBuffers();
}

void update(int value)
{
  angle += 2.0f;
  if (angle > 360.0f)
  {
    angle -= 360.0f;
  }

  glutPostRedisplay();
  glutTimerFunc(16, update, 0); // 60 FPS approximately
}

void reshape(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  float aspect = (float)w / (float)h;
  if (w >= h)
  {
    glOrtho(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0, -1.0, 1.0);
  }
  else
  {
    glOrtho(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect, -1.0, 1.0);
  }

  glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Enable depth buffer
  glutInitWindowSize(800, 800);
  glutCreateWindow("Windows 95 Logo");

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST); // Enable depth testing

  // Set up perspective projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0f, 1.0f, 0.1f, 100.0f);
  glMatrixMode(GL_MODELVIEW);

  glutDisplayFunc(display);
  glutTimerFunc(0, update, 0);

  glutMainLoop();
  return 0;
}