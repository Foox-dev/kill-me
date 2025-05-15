#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

// Color states
typedef struct
{
  float r, g, b;
  const char *name;
} ColorState;

ColorState colors[] = {
    {0.4f, 0.6f, 1.0f, "Blue Ball"},   // Blue
    {1.0f, 0.4f, 0.4f, "Red Ball"},    // Red
    {0.4f, 1.0f, 0.4f, "Green Ball"},  // Green
    {1.0f, 1.0f, 0.4f, "Yellow Ball"}, // Yellow
};

// Game state
typedef struct
{
  long long int score;
  int clickPower;
  float clickSize;
  int colorIndex;
} GameState;

GameState game = {
    .score = 0,
    .clickPower = 10,
    .clickSize = 0.3f,
    .colorIndex = 0};

void renderText(float x, float y, const char *text)
{
  glRasterPos2f(x, y);
  for (const char *c = text; *c != '\0'; c++)
  {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
  }
}

void drawClickButton()
{
  ColorState currentColor = colors[game.colorIndex];
  glColor3f(currentColor.r, currentColor.g, currentColor.b);

  // Draw circular button
  glBegin(GL_POLYGON);
  int numSegments = 32;
  for (int i = 0; i < numSegments; i++)
  {
    float theta = 2.0f * 3.1415926f * (float)i / (float)numSegments;
    float x = game.clickSize * cosf(theta);
    float y = game.clickSize * sinf(theta);
    glVertex2f(x, y);
  }
  glEnd();
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT);
  glLoadIdentity();

  // Draw the click button
  drawClickButton();

  // Display score
  glColor3f(0.0f, 0.0f, 0.0f);
  char scoreText[32];
  sprintf(scoreText, "Score: %lld", game.score);
  renderText(-0.9f, 0.9f, scoreText);

  glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    // Convert screen coordinates to OpenGL coordinates
    float glX = (2.0f * x) / glutGet(GLUT_WINDOW_WIDTH) - 1.0f;
    float glY = 1.0f - (2.0f * y) / glutGet(GLUT_WINDOW_HEIGHT);

    // Check if click is within the button (basic circle collision)
    if ((glX * glX + glY * glY) < (game.clickSize * game.clickSize))
    {
      game.score += game.clickPower;

      // Check if we need to change color (every 100 clicks)
      if (game.score % 100 == 0)
      {
        game.colorIndex = (game.colorIndex + 1) % (sizeof(colors) / sizeof(colors[0]));
        glutSetWindow(glutGetWindow());
        glutSetWindowTitle(colors[game.colorIndex].name);
      }

      glutPostRedisplay();
    }
  }
}

void reshape(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // Maintain aspect ratio
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
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(800, 600);
  glutCreateWindow("Blue Ball");

  glClearColor(0.9f, 0.9f, 0.9f, 1.0f); // Light gray background

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);

  glutMainLoop();
  return 0;
}