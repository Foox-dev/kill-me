// gcc spinningsquare.c -o spinningsquare -lglut -lGL -lGLU -lm

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

float angle = 0.0f;

// Windows 95 logo colors
GLfloat red[] = {0.87f, 0.0f, 0.0f};
GLfloat green[] = {0.0f, 0.87f, 0.0f};
GLfloat blue[] = {0.0f, 0.0f, 0.87f};
GLfloat yellow[] = {0.87f, 0.87f, 0.0f};

float cameraX = 0.0f;
float cameraY = 0.0f;
float cameraZ = -2.0f;
float tiltAngle = 15.0f;
float rotationSpeed = 2.0f;

int mouseX = 0;
int mouseY = 0;
int rightMouseDown = 0;

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

void keyboard(unsigned char key, int x, int y)
{
  float moveSpeed = 0.01f;
  switch (key)
  {
  case 'w':
    cameraY += moveSpeed;
    break;
  case 's':
    cameraY -= moveSpeed;
    break;
  case 'a':
    cameraX -= moveSpeed;
    break;
  case 'd':
    cameraX += moveSpeed;
    break;
  }
  glutPostRedisplay();
}

void specialKeys(int key, int x, int y)
{
  switch (key)
  {
  case GLUT_KEY_LEFT:
    rotationSpeed = fmax(0.0f, rotationSpeed - 0.5f);
    break;
  case GLUT_KEY_RIGHT:
    rotationSpeed = fmin(100.0f, rotationSpeed + 0.5f);
    break;
  case GLUT_KEY_UP:
    tiltAngle = fmin(89.0f, tiltAngle + 5.0f);
    break;
  case GLUT_KEY_DOWN:
    tiltAngle = fmax(-89.0f, tiltAngle - 5.0f);
    break;
  }
  glutPostRedisplay();
}

//
void mouse(int button, int state, int x, int y)
{
  float zoomSpeed = 0.1f;

  if (button == GLUT_RIGHT_BUTTON)
  {
    rightMouseDown = (state == GLUT_DOWN);
    mouseX = x;
    mouseY = y;
  }
  else if (state == GLUT_UP)
  {
    return;
  }

  switch (button)
  {
  case 3: // Mouse wheel up
    cameraZ += zoomSpeed;
    break;
  case 4: // Mouse wheel down
    cameraZ -= zoomSpeed;
    break;
  }
  glutPostRedisplay();
}

void mouseMotion(int x, int y)
{
  if (rightMouseDown)
  {
    float sensitivity = 0.5f;
    int deltaX = x - mouseX;
    int deltaY = y - mouseY;

    angle += deltaX * sensitivity;
    if (angle > 360.0f)
      angle -= 360.0f;
    if (angle < 0.0f)
      angle += 360.0f;

    tiltAngle += deltaY * sensitivity;
    tiltAngle = fmax(-89.0f, fmin(89.0f, tiltAngle));

    glutPostRedisplay();
  }
  mouseX = x;
  mouseY = y;
}

void renderText(float x, float y, const char *text)
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT));

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glRasterPos2f(x, y);

  // Disable depth testing temporarily
  glDisable(GL_DEPTH_TEST);

  // Set text color (black in this case)
  glColor3f(0.0f, 0.0f, 0.0f);


  for (const char *c = text; *c != '\0'; c++)
  {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
  }

  glEnable(GL_DEPTH_TEST);

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  // Update camera position and rotation
  glTranslatef(cameraX, cameraY, cameraZ);
  glRotatef(tiltAngle, 1.0f, 0.0f, 0.0f);

  glPushMatrix();
  glRotatef(angle, 0.0f, 1.0f, 0.0f);
  drawWindowsLogo();
  glPopMatrix();

  // Add text overlay
  char buffer[128];
  sprintf(buffer, "Hello");
  renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 20, buffer);

  glutSwapBuffers();
}

void update(int value)
{
  angle += rotationSpeed;
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
  glutCreateWindow("Spinning Windows 95 Logo");

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST); // Enable depth testing

  // Set up perspective projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0f, 1.0f, 0.1f, 100.0f);
  glMatrixMode(GL_MODELVIEW);

  glutDisplayFunc(display);
  glutTimerFunc(0, update, 0);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(specialKeys);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion); // Add this line

  glutMainLoop();
  return 0;
}