#include <GL/glut.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

// Function prototypes
void spawnEnemy(void);
void spawnMinion();
bool checkCircleCollision(float x1, float y1, float r1, float x2, float y2, float r2); // New prototype

#define MAP_WIDTH 20
#define MAP_HEIGHT 20
#define TILE_SIZE 0.1f
#define MAX_ENEMIES 100
#define ENEMY_SIZE 0.04f
#define NORMAL_SPEED 0.01f    // Reduced from 0.05f
#define DIAGONAL_SPEED 0.007f // Reduced, approximately NORMAL_SPEED / sqrt(2)
#define PI 3.14159265358979323846
#define ATTACK_RANGE 0.2f // Increased from 0.1f
#define ATTACK_ARC 90.0f  // Increased from 45.0f - wider attack arc
#define ENEMY_CHASE_RANGE 0.5f
#define ENEMY_ATTACK_RANGE 0.08f
#define ENEMY_SPEED 0.005f         // New constant for enemy movement
#define ENEMY_ATTACK_COOLDOWN 1.0f // Time between enemy attacks
#define ENEMY_DAMAGE 10            // Damage dealt by enemies
#define PLAYER_ATTACK_DAMAGE 20    // Damage dealt by player
#define ROTATION_SPEED 5.0f        // Degrees per frame
#define BLAST_RANGE 0.25f          // 2.5 tiles radius
#define ENEMY_MAX_HEALTH 100
#define GOLD_DROP_AMOUNT 10
#define GOLD_SIZE 0.02f
#define SPAWN_INTERVAL 5.0f      // Time between enemy spawns in seconds
#define WAVE_INCREASE_TIME 30.0f // Time between difficulty increases
#define MAX_ENEMIES_PER_WAVE 5   // Maximum enemies that can spawn per wave
#define BOSS_WAVE_INTERVAL 5     // Boss spawns every 5 waves
#define BOSS_SIZE 0.08f          // Double normal enemy size
#define BOSS_HEALTH 500          // Base boss health
#define BOSS_DAMAGE 20           // Boss damage
#define BOSS_BLAST_RANGE 0.3f    // Boss blast range
#define BOSS_BLAST_COOLDOWN 3.0f // Time between boss blasts
#define BOSS_VIEW_RANGE 1.0f     // Double normal enemy chase range
#define BOSS_MINIONS 3           // Number of minions to spawn with boss

// Game entities
typedef struct
{
  float x, y;
  float speed;
  int health;
  float rotation; // In degrees
  bool attacking;
  float attackTimer;
  bool rightMouseDown; // Add this line
  int gold;            // Add this field
  int mana;            // Add mana field
} Player;

typedef struct
{
  float x, y;
  int health;
  bool active;
  float speed;
  float timeSinceLastAttack;
  bool isAttacking;
  bool isBoss;         // Add this
  float blastCooldown; // Add this
} Enemy;

// Map tiles
typedef enum
{
  TILE_GRASS,
  TILE_WALL,
  TILE_WATER
} TileType;

// Game state
typedef struct
{
  Player player;
  Enemy enemies[MAX_ENEMIES];
  TileType map[MAP_WIDTH][MAP_HEIGHT];
  bool keys[256];        // Track keyboard state
  bool showDebug;        // Toggle for debug info
  bool specialKeys[256]; // Add this to track special key state
  bool waveActive;
  float spawnTimer;
  float waveTimer;
  int currentWave;
  int enemiesThisWave;
  bool paused; // Add pause field
} GameState;

GameState game = {
    .player = {0.0f, 0.0f, NORMAL_SPEED, 100, 0.0f, false, 0.0f, false, 0, 100}, // Initialize mana to 100
    .showDebug = false,
    .specialKeys = {false},
    .waveActive = false,
    .spawnTimer = 0,
    .waveTimer = 0,
    .currentWave = 0,
    .enemiesThisWave = 0,
    .paused = false};

// Colors for different tiles
const float tileColors[][3] = {
    {0.2f, 0.8f, 0.2f}, // Grass
    {0.5f, 0.5f, 0.5f}, // Wall
    {0.0f, 0.4f, 0.8f}  // Water
};

void initMap()
{
  // Initialize a simple map with walls around the edges
  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      if (x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1)
        game.map[x][y] = TILE_WALL;
      else
        game.map[x][y] = TILE_GRASS;
    }
  }
}

void initEnemies()
{
  for (int i = 0; i < MAX_ENEMIES; i++)
  {
    game.enemies[i].active = false; // Initialize all enemies as inactive
    game.enemies[i].health = 0;
  }
}

void drawTile(int x, int y)
{
  float worldX = (x - MAP_WIDTH / 2) * TILE_SIZE;
  float worldY = (y - MAP_HEIGHT / 2) * TILE_SIZE;

  const float *color = tileColors[game.map[x][y]];
  glColor3f(color[0], color[1], color[2]);

  glBegin(GL_QUADS);
  glVertex2f(worldX, worldY);
  glVertex2f(worldX + TILE_SIZE, worldY);
  glVertex2f(worldX + TILE_SIZE, worldY + TILE_SIZE);
  glVertex2f(worldX, worldY + TILE_SIZE);
  glEnd();
}

void drawPlayer()
{
  glPushMatrix();
  glTranslatef(game.player.x, game.player.y, 0.0f);
  glRotatef(game.player.rotation, 0.0f, 0.0f, 1.0f);

  // Draw player triangle
  glColor3f(1.0f, 0.0f, 0.0f);
  glBegin(GL_TRIANGLES);
  glVertex2f(-0.03f, -0.03f);
  glVertex2f(0.03f, -0.03f);
  glVertex2f(0.0f, 0.06f);
  glEnd();

  // Draw attack animation when attacking
  if (game.player.attacking)
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (game.specialKeys[GLUT_KEY_DOWN])
    {
      // Only draw blast wave for down key
      glColor4f(1.0f, 0.4f, 0.2f, 0.5f * (game.player.attackTimer / 0.2f));
      glBegin(GL_TRIANGLE_FAN);
      glVertex2f(0.0f, 0.0f);
      for (int i = 0; i <= 32; i++)
      {
        float angle = i * 2 * PI / 32;
        glVertex2f(cos(angle) * BLAST_RANGE, sin(angle) * BLAST_RANGE);
      }
      glEnd();
    }
    else if (game.specialKeys[GLUT_KEY_UP])
    {
      // Only draw attack arc for up key
      // Draw forward attack arc
      glColor4f(0.2f, 0.4f, 1.0f, 0.5f * (game.player.attackTimer / 0.2f));
      glBegin(GL_TRIANGLE_FAN);
      glVertex2f(0.0f, 0.0f);
      float startAngle = -ATTACK_ARC / 2;
      float endAngle = ATTACK_ARC / 2;
      for (int i = 0; i <= 32; i++)
      {
        float angle = startAngle + (endAngle - startAngle) * i / 32;
        float rad = (angle + 90) * PI / 180.0f;
        glVertex2f(cos(rad) * ATTACK_RANGE, sin(rad) * ATTACK_RANGE);
      }
      glEnd();
    }
    glDisable(GL_BLEND);
  }

  glPopMatrix();
}

void drawEnemies()
{
  for (int i = 0; i < MAX_ENEMIES; i++)
  {
    if (!game.enemies[i].active || game.enemies[i].health <= 0)
      continue;

    float healthPercent = game.enemies[i].health /
                          (float)(game.enemies[i].isBoss ? BOSS_HEALTH : ENEMY_MAX_HEALTH);

    // Darken color as health decreases
    glColor3f(0.8f * healthPercent, 0.2f * healthPercent, 0.2f * healthPercent);

    // Draw enemy
    glPushMatrix();
    glTranslatef(game.enemies[i].x, game.enemies[i].y, 0.0f);

    if (game.enemies[i].isBoss)
    {
      // Draw boss (larger, different color)
      glColor3f(0.8f * healthPercent, 0.2f, 0.8f * healthPercent);
      glBegin(GL_QUADS);
      glVertex2f(-BOSS_SIZE, -BOSS_SIZE);
      glVertex2f(BOSS_SIZE, -BOSS_SIZE);
      glVertex2f(BOSS_SIZE, BOSS_SIZE);
      glVertex2f(-BOSS_SIZE, BOSS_SIZE);
      glEnd();
    }
    else
    {
      glBegin(GL_QUADS);
      glVertex2f(-ENEMY_SIZE, -ENEMY_SIZE);
      glVertex2f(ENEMY_SIZE, -ENEMY_SIZE);
      glVertex2f(ENEMY_SIZE, ENEMY_SIZE);
      glVertex2f(-ENEMY_SIZE, ENEMY_SIZE);
      glEnd();
    }

    // Health bar (adjusted for boss size if needed)
    float size = game.enemies[i].isBoss ? BOSS_SIZE : ENEMY_SIZE;
    glColor3f(1.0f - healthPercent, healthPercent, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-size, size + 0.01f);
    glVertex2f(-size + (size * 2 * healthPercent), size + 0.01f);
    glVertex2f(-size + (size * 2 * healthPercent), size + 0.02f);
    glVertex2f(-size, size + 0.02f);
    glEnd();

    glPopMatrix();
  }
}

void updatePlayer()
{
  if (game.paused)
    return; // Don't update if paused

  float newX = game.player.x;
  float newY = game.player.y;
  bool movingDiagonal = false;

  // Check if moving diagonally
  if ((game.keys['w'] || game.keys['s']) && (game.keys['a'] || game.keys['d']))
  {
    movingDiagonal = true;
  }

  float currentSpeed = movingDiagonal ? DIAGONAL_SPEED : NORMAL_SPEED;

  // Update position based on keys
  if (game.keys['w'])
    newY += currentSpeed;
  if (game.keys['s'])
    newY -= currentSpeed;
  if (game.keys['a'])
    newX -= currentSpeed;
  if (game.keys['d'])
    newX += currentSpeed;

  // Check wall collision
  int mapX = (int)((newX / TILE_SIZE) + (MAP_WIDTH / 2));
  int mapY = (int)((newY / TILE_SIZE) + (MAP_HEIGHT / 2));

  bool canMove = true;

  // Check collision with enemies
  for (int i = 0; i < MAX_ENEMIES; i++)
  {
    if (game.enemies[i].active)
    {
      if (checkCircleCollision(newX, newY, ENEMY_SIZE,
                               game.enemies[i].x, game.enemies[i].y, ENEMY_SIZE))
      {
        canMove = false;
        break;
      }
    }
  }

  // Move only if no collisions and not in wall
  if (canMove && mapX >= 0 && mapX < MAP_WIDTH && mapY >= 0 && mapY < MAP_HEIGHT)
  {
    if (game.map[mapX][mapY] != TILE_WALL)
    {
      game.player.x = newX;
      game.player.y = newY;
    }
  }
}

void updateEnemies()
{
  for (int i = 0; i < MAX_ENEMIES; i++)
  {
    if (!game.enemies[i].active)
      continue;

    float viewRange = game.enemies[i].isBoss ? BOSS_VIEW_RANGE : ENEMY_CHASE_RANGE;

    // Calculate distance to player
    float dx = game.player.x - game.enemies[i].x;
    float dy = game.player.y - game.enemies[i].y;
    float distance = sqrt(dx * dx + dy * dy);

    // Chase player if within range
    if (distance < viewRange && distance > ENEMY_ATTACK_RANGE)
    {
      // Calculate movement direction
      float angle = atan2(dy, dx);
      float newX = game.enemies[i].x + cos(angle) * game.enemies[i].speed;
      float newY = game.enemies[i].y + sin(angle) * game.enemies[i].speed;

      // Check wall collision
      int mapX = (newX / TILE_SIZE) + (MAP_WIDTH / 2);
      int mapY = (newY / TILE_SIZE) + (MAP_HEIGHT / 2);

      bool canMove = true;

      // Check collision with other enemies
      for (int j = 0; j < MAX_ENEMIES; j++)
      {
        if (j != i && game.enemies[j].active)
        {
          if (checkCircleCollision(newX, newY, ENEMY_SIZE,
                                   game.enemies[j].x, game.enemies[j].y, ENEMY_SIZE))
          {
            canMove = false;
            break;
          }
        }
      }

      // Move only if no collisions and not in wall
      if (canMove && mapX >= 0 && mapX < MAP_WIDTH && mapY >= 0 && mapY < MAP_HEIGHT)
      {
        if (game.map[mapX][mapY] != TILE_WALL)
        {
          game.enemies[i].x = newX;
          game.enemies[i].y = newY;
        }
      }
    }

    // Attack logic
    if (distance < ENEMY_ATTACK_RANGE)
    {
      if (game.enemies[i].timeSinceLastAttack >= ENEMY_ATTACK_COOLDOWN)
      {
        game.player.health -= ENEMY_DAMAGE;
        game.enemies[i].timeSinceLastAttack = 0;
        game.enemies[i].isAttacking = true;
      }
    }

    // Update attack cooldown
    if (game.enemies[i].timeSinceLastAttack < ENEMY_ATTACK_COOLDOWN)
    {
      game.enemies[i].timeSinceLastAttack += 0.016f;
      game.enemies[i].isAttacking = false;
    }

    // Boss special abilities
    if (game.enemies[i].isBoss)
    {
      game.enemies[i].blastCooldown += 0.016f;

      if (game.enemies[i].blastCooldown >= BOSS_BLAST_COOLDOWN)
      {
        // Perform boss blast attack
        float dx = game.player.x - game.enemies[i].x;
        float dy = game.player.y - game.enemies[i].y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance <= BOSS_BLAST_RANGE)
        {
          game.player.health -= BOSS_DAMAGE;
          printf("Boss blast hit player! Player health: %d\n", game.player.health);
        }

        game.enemies[i].blastCooldown = 0;
      }
    }
  }
}

void renderText(float x, float y, const char *text)
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT), -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glRasterPos2f(x, y);

  for (const char *c = text; *c != '\0'; c++)
  {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
  }

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

void drawDebugInfo()
{
  if (!game.showDebug)
    return;

  char buffer[128];
  glColor3f(1.0f, 1.0f, 1.0f);

  // Player stats
  sprintf(buffer, "Health: %d", game.player.health);
  renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 20, buffer);

  sprintf(buffer, "Gold: %d", game.player.gold);
  renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 40, buffer);

  sprintf(buffer, "Position: (%.2f, %.2f)", game.player.x, game.player.y);
  renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 60, buffer);

  sprintf(buffer, "Rotation: %.1f°", game.player.rotation);
  renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 80, buffer);

  // Count active enemies
  int activeEnemies = 0;
  for (int i = 0; i < MAX_ENEMIES; i++)
  {
    if (game.enemies[i].active)
      activeEnemies++;
  }
  sprintf(buffer, "Active Enemies: %d", activeEnemies);
  renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 100, buffer);

  if (game.waveActive)
  {
    sprintf(buffer, "Wave: %d", game.currentWave + 1);
    renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 120, buffer);

    sprintf(buffer, "Next Spawn: %.1fs", SPAWN_INTERVAL - game.spawnTimer);
    renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 140, buffer);

    sprintf(buffer, "Wave Timer: %.1fs", WAVE_INCREASE_TIME - game.waveTimer);
    renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 160, buffer);
  }

  // Add boss information
  bool bossAlive = false;
  int bossIndex = -1;
  for (int i = 0; i < MAX_ENEMIES; i++)
  {
    if (game.enemies[i].active && game.enemies[i].isBoss)
    {
      bossAlive = true;
      bossIndex = i;
      break;
    }
  }

  if (bossAlive)
  {
    sprintf(buffer, "Boss Health: %d", game.enemies[bossIndex].health);
    renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 180, buffer);

    sprintf(buffer, "Boss Position: (%.2f, %.2f)",
            game.enemies[bossIndex].x, game.enemies[bossIndex].y);
    renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 200, buffer);

    sprintf(buffer, "Boss Blast Cooldown: %.1f",
            BOSS_BLAST_COOLDOWN - game.enemies[bossIndex].blastCooldown);
    renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 220, buffer);
  }

  // Draw enemy ranges
  for (int i = 0; i < MAX_ENEMIES; i++)
  {
    if (!game.enemies[i].active)
      continue;

    glPushMatrix();
    glTranslatef(game.enemies[i].x, game.enemies[i].y, 0.0f);

    // Chase range
    glColor4f(1.0f, 1.0f, 0.0f, 0.2f);
    glBegin(GL_LINE_LOOP);
    for (int j = 0; j < 32; j++)
    {
      float angle = j * 2 * PI / 32;
      glVertex2f(cos(angle) * ENEMY_CHASE_RANGE,
                 sin(angle) * ENEMY_CHASE_RANGE);
    }
    glEnd();

    glPopMatrix();
  }

  // Draw attack range circle around player
  glPushMatrix();
  glTranslatef(game.player.x, game.player.y, 0.0f);
  glColor4f(1.0f, 0.0f, 0.0f, 0.2f);
  glBegin(GL_LINE_LOOP);
  for (int i = 0; i < 32; i++)
  {
    float angle = i * 2 * PI / 32;
    glVertex2f(cos(angle) * ATTACK_RANGE, sin(angle) * ATTACK_RANGE);
  }
  glEnd();
  glPopMatrix();
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT);
  glLoadIdentity();

  // Draw map
  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      drawTile(x, y);
      // Draw tile coordinates in debug mode
      if (game.showDebug)
      {
        char coords[10];
        sprintf(coords, "%d,%d", x, y);
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f((x - MAP_WIDTH / 2) * TILE_SIZE + 0.01f,
                      (y - MAP_HEIGHT / 2) * TILE_SIZE + 0.01f);
        for (char *c = coords; *c != '\0'; c++)
        {
          glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
        }
      }
    }
  }

  drawEnemies();
  drawPlayer();

  // Draw wave counter in top left (after all other drawing)
  char buffer[128];
  glColor3f(1.0f, 1.0f, 1.0f);
  if (game.waveActive)
  {
    sprintf(buffer, "Wave: %d", game.currentWave + 1);
    renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 20, buffer);

    sprintf(buffer, "Next Enemy: %.1fs", SPAWN_INTERVAL - game.spawnTimer);
    renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 40, buffer);
  }
  else
  {
    renderText(10, glutGet(GLUT_WINDOW_HEIGHT) - 20, "Press 2 to start waves");
  }

  drawDebugInfo();

  // Draw mana bar in top right
  glColor3f(0.0f, 0.0f, 0.0f); // Black border
  glBegin(GL_LINE_LOOP);
  glVertex2f(0.8f, 0.9f);
  glVertex2f(0.95f, 0.9f);
  glVertex2f(0.95f, 0.95f);
  glVertex2f(0.8f, 0.95f);
  glEnd();

  // Draw mana fill
  float manaPercent = game.player.mana / 100.0f;
  glColor3f(0.0f, 0.0f, 1.0f); // Blue for mana
  glBegin(GL_QUADS);
  glVertex2f(0.8f, 0.9f);
  glVertex2f(0.8f + 0.15f * manaPercent, 0.9f);
  glVertex2f(0.8f + 0.15f * manaPercent, 0.95f);
  glVertex2f(0.8f, 0.95f);
  glEnd();

  // Draw mana text
  sprintf(buffer, "Mana: %d/100", game.player.mana);
  glColor3f(1.0f, 1.0f, 1.0f);
  renderText(glutGet(GLUT_WINDOW_WIDTH) - 100,
             glutGet(GLUT_WINDOW_HEIGHT) - 20, buffer);

  // Draw pause overlay if paused
  if (game.paused)
  {
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(1.0f, 1.0f, 1.0f);
    renderText(glutGet(GLUT_WINDOW_WIDTH) / 2 - 30,
               glutGet(GLUT_WINDOW_HEIGHT) / 2, "PAUSED");
  }

  glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
  if (key >= 'A' && key <= 'Z')
  {
    // Convert uppercase to lowercase
    key = key + ('a' - 'A');
  }
  game.keys[key] = true;

  if (key == 27)
  { // ASCII for escape key
    game.paused = !game.paused;
    printf("Game %s\n", game.paused ? "Paused" : "Resumed");
  }
  else if (key == '\t')
  {
    game.showDebug = !game.showDebug;
  }
  else if (key == '1')
  {
    printf("Spawning enemy with '1' key\n");
    spawnEnemy();
  }
  else if (key == '@')
  { // Shift+2 for manual boss spawn
    // Force spawn a boss
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
      if (!game.enemies[i].active)
      {
        game.enemies[i].active = true;
        game.enemies[i].isBoss = true;
        game.enemies[i].health = BOSS_HEALTH;
        game.enemies[i].speed = ENEMY_SPEED * 0.7f;
        game.enemies[i].blastCooldown = 0;

        // Random spawn position
        int x = 1 + (rand() % (MAP_WIDTH - 2));
        int y = 1 + (rand() % (MAP_HEIGHT - 2));
        game.enemies[i].x = (x - MAP_WIDTH / 2) * TILE_SIZE;
        game.enemies[i].y = (y - MAP_HEIGHT / 2) * TILE_SIZE;

        // Spawn minions
        for (int j = 0; j < BOSS_MINIONS; j++)
        {
          spawnMinion();
        }

        printf("Manually spawned a boss!\n");
        break;
      }
    }
  }
  else if (key == '2')
  {
    if (!game.waveActive)
    {
      game.waveActive = true;
      game.currentWave = 0;
      game.spawnTimer = SPAWN_INTERVAL; // Spawn first enemy immediately
      game.waveTimer = 0;
      game.enemiesThisWave = 0;
      printf("Wave mode started!\n");
    }
  }
  else if (key == '3' && game.waveActive)
  {
    // Skip to next wave
    game.currentWave++;
    game.waveTimer = 0;
    game.enemiesThisWave = 0;
    game.spawnTimer = SPAWN_INTERVAL; // Reset spawn timer for immediate enemy

    // Kill all current enemies
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
      if (game.enemies[i].active)
      {
        game.enemies[i].active = false;
      }
    }

    printf("Skipped to Wave %d!\n", game.currentWave + 1);
  }
}

// Add special key handler for F1
void specialKeysUp(int key, int x, int y)
{
  game.specialKeys[key] = false;
}

void specialKeys(int key, int x, int y)
{
  game.specialKeys[key] = true;

  if (key == GLUT_KEY_F1)
  {
    printf("Spawning enemy with F1 key\n");
    spawnEnemy();
  }
  else if (key == GLUT_KEY_UP && !game.player.attacking)
  {
    // Forward attack
    game.player.attacking = true;
    game.player.attackTimer = 0.2f;

    // Check for enemies in attack arc
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
      if (!game.enemies[i].active || game.enemies[i].health <= 0)
        continue;

      float dx = game.enemies[i].x - game.player.x;
      float dy = game.enemies[i].y - game.player.y;
      float distance = sqrt(dx * dx + dy * dy);

      if (distance <= ATTACK_RANGE)
      {
        float angleToEnemy = atan2(dy, dx) * 180.0f / PI;
        float playerAngle = game.player.rotation;
        float angleDiff = angleToEnemy - playerAngle;

        // Normalize angle difference to [-180, 180]
        while (angleDiff > 180.0f)
          angleDiff -= 360.0f;
        while (angleDiff < -180.0f)
          angleDiff += 360.0f;
        angleDiff = fabs(angleDiff - 90); // Adjust by 90 degrees to match visual

        if (angleDiff <= ATTACK_ARC / 2 || distance <= ATTACK_RANGE * 0.5f)
        {
          game.enemies[i].health -= PLAYER_ATTACK_DAMAGE;
          printf("Forward attack hit! Enemy %d health: %d\n", i, game.enemies[i].health);

          if (game.enemies[i].health <= 0)
          {
            game.enemies[i].active = false;
            game.player.gold += GOLD_DROP_AMOUNT;
            // Add mana on any enemy kill
            game.player.mana = (game.player.mana + 10 <= 100) ? game.player.mana + 10 : 100;
            if (game.enemies[i].isBoss)
            {
              game.player.mana = 100;                   // Full mana restore on boss kill
              game.player.gold += GOLD_DROP_AMOUNT * 5; // Extra gold for boss
            }
            printf("Enemy defeated! Got %d gold, +10 mana. Total: %d gold, %d mana\n",
                   GOLD_DROP_AMOUNT, game.player.gold, game.player.mana);
          }
        }
      }
    }
  }
  else if (key == GLUT_KEY_DOWN && !game.player.attacking)
  {
    // Blast attack
    // Only allow blast if enough mana
    if (game.player.mana >= 20)
    {
      game.player.attacking = true;
      game.player.attackTimer = 0.2f;
      game.player.mana -= 20; // Use mana for blast

      // Check all enemies for blast damage
      for (int i = 0; i < MAX_ENEMIES; i++)
      {
        if (!game.enemies[i].active || game.enemies[i].health <= 0)
          continue;

        float dx = game.enemies[i].x - game.player.x;
        float dy = game.enemies[i].y - game.player.y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance <= BLAST_RANGE)
        {
          game.enemies[i].health -= PLAYER_ATTACK_DAMAGE;
          printf("Blast hit! Enemy %d health: %d\n", i, game.enemies[i].health);

          if (game.enemies[i].health <= 0)
          {
            game.enemies[i].active = false;
            game.player.gold += GOLD_DROP_AMOUNT;
            game.player.mana = (game.player.mana + 10 <= 100) ? game.player.mana + 10 : 100;
            if (game.enemies[i].isBoss)
            {
              game.player.mana = 100;                   // Full mana restore on boss kill
              game.player.gold += GOLD_DROP_AMOUNT * 5; // Extra gold for boss
            }
            printf("Enemy defeated! Got %d gold, +10 mana. Total: %d gold, %d mana\n",
                   GOLD_DROP_AMOUNT, game.player.gold, game.player.mana);
          }
        }
      }
    }
    else
    {
      printf("Not enough mana for blast attack!\n");
    }
  }
}
void keyboardUp(unsigned char key, int x, int y)
{
  if (key >= 'A' && key <= 'Z')
  {
    // Convert uppercase to lowercase
    key = key + ('a' - 'A');
  }

  game.keys[key] = false; // Make sure to set key to false when released
}

void mouseMove(int x, int y)
{
  // Only update rotation when right mouse is held
  if (!game.player.rightMouseDown)
  {
    return;
  }

  // Convert window coordinates to OpenGL coordinates
  float screenX = (2.0f * x / glutGet(GLUT_WINDOW_WIDTH) - 1.0f);
  float screenY = -(2.0f * y / glutGet(GLUT_WINDOW_HEIGHT) - 1.0f);

  // Calculate angle between player and mouse
  float dx = screenX - game.player.x;
  float dy = screenY - game.player.y;

  // Only update rotation if there's significant mouse movement
  if (fabs(dx) > 0.001f || fabs(dy) > 0.001f)
  {
    float targetRotation = atan2(dy, dx) * 180.0f / PI;

    // Normalize target rotation to [0, 360)
    while (targetRotation < 0)
      targetRotation += 360.0f;
    while (targetRotation >= 360.0f)
      targetRotation -= 360.0f;

    game.player.rotation = targetRotation;
  }
}

void mouse(int button, int state, int x, int y)
{
  if (button == GLUT_RIGHT_BUTTON)
  {
    game.player.rightMouseDown = (state == GLUT_DOWN);
  }
  else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    game.player.attacking = true;
    game.player.attackTimer = 0.2f;

    // Check for enemies in attack arc
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
      if (!game.enemies[i].active || game.enemies[i].health <= 0)
        continue;

      float dx = game.enemies[i].x - game.player.x;
      float dy = game.enemies[i].y - game.player.y;
      float distance = sqrt(dx * dx + dy * dy);

      if (distance <= ATTACK_RANGE)
      {
        float angleToEnemy = atan2(dy, dx) * 180.0f / PI;
        float playerAngle = game.player.rotation;
        float angleDiff = angleToEnemy - playerAngle;

        // Normalize angle difference to [-180, 180]
        while (angleDiff > 180.0f)
          angleDiff -= 360.0f;
        while (angleDiff < -180.0f)
          angleDiff += 360.0f;
        angleDiff = fabs(angleDiff - 90); // Adjust by 90 degrees to match visual

        // More generous hit detection for close range
        if (angleDiff <= ATTACK_ARC / 2 || distance <= ATTACK_RANGE * 0.5f)
        {
          game.enemies[i].health -= PLAYER_ATTACK_DAMAGE;
          printf("Hit! Enemy %d health: %d\n", i, game.enemies[i].health);

          if (game.enemies[i].health <= 0)
          {
            game.enemies[i].active = false;
            game.player.gold += GOLD_DROP_AMOUNT;
            game.player.mana = (game.player.mana + 10 <= 100) ? game.player.mana + 10 : 100;
            if (game.enemies[i].isBoss)
            {
              game.player.mana = 100;                   // Full mana restore on boss kill
              game.player.gold += GOLD_DROP_AMOUNT * 5; // Extra gold for boss
            }
            printf("Enemy defeated! Got %d gold, +10 mana. Total: %d gold, %d mana\n",
                   GOLD_DROP_AMOUNT, game.player.gold, game.player.mana);
          }
        }
      }
    }
  }
}

void spawnEnemy()
{
  for (int i = 0; i < MAX_ENEMIES; i++)
  {
    if (!game.enemies[i].active)
    {
      // Check if this should be a boss wave
      bool isBoss = ((game.currentWave + 1) % BOSS_WAVE_INTERVAL) == 0;

      // Spawn location code remains the same
      int x = 1 + (rand() % (MAP_WIDTH - 2));
      int y = 1 + (rand() % (MAP_HEIGHT - 2));

      game.enemies[i].x = (x - MAP_WIDTH / 2) * TILE_SIZE;
      game.enemies[i].y = (y - MAP_HEIGHT / 2) * TILE_SIZE;
      game.enemies[i].active = true;
      game.enemies[i].isBoss = isBoss;

      if (isBoss)
      {
        game.enemies[i].health = BOSS_HEALTH + (game.currentWave * 100);
        game.enemies[i].speed = ENEMY_SPEED * 0.7f; // Slower but stronger
        game.enemies[i].blastCooldown = 0;

        // Spawn minions around the boss
        for (int j = 0; j < BOSS_MINIONS; j++)
        {
          spawnEnemy(); // Spawn regular enemies as minions
        }

        printf("Spawned BOSS enemy! (Health: %d)\n", game.enemies[i].health);
      }
      else
      {
        game.enemies[i].health = 100 + (game.currentWave * 20);
        game.enemies[i].speed = ENEMY_SPEED * (1.0f + game.currentWave * 0.1f);
      }

      game.enemies[i].timeSinceLastAttack = 0;
      return;
    }
  }
  printf("Max enemies reached, cannot spawn more\n");
}

void spawnMinion()
{
  for (int i = 0; i < MAX_ENEMIES; i++)
  {
    if (!game.enemies[i].active)
    {
      // Spawn near boss
      float angle = (rand() % 360) * PI / 180.0f;
      float distance = 0.2f + (rand() % 100) / 100.0f * 0.3f; // Random distance from boss

      game.enemies[i].x = game.enemies[0].x + cos(angle) * distance;
      game.enemies[i].y = game.enemies[0].y + sin(angle) * distance;
      game.enemies[i].active = true;
      game.enemies[i].isBoss = false;
      game.enemies[i].health = 150;               // Stronger than normal enemies
      game.enemies[i].speed = ENEMY_SPEED * 1.2f; // Faster than normal enemies
      game.enemies[i].timeSinceLastAttack = 0;
      printf("Spawned boss minion at (%.2f, %.2f)\n", game.enemies[i].x, game.enemies[i].y);
      return;
    }
  }
}

void update(int value)
{
  if (!game.paused)
  {
    // Handle rotation
    if (game.specialKeys[GLUT_KEY_RIGHT])
    {
      game.player.rotation -= ROTATION_SPEED;
      if (game.player.rotation < 0)
        game.player.rotation += 360.0f;
    }
    if (game.specialKeys[GLUT_KEY_LEFT])
    {
      game.player.rotation += ROTATION_SPEED;
      if (game.player.rotation >= 360.0f)
        game.player.rotation -= 360.0f;
    }

    updatePlayer();  // Make sure this is called
    updateEnemies(); // Update enemies after player

    // Update attack timer
    if (game.player.attacking)
    {
      game.player.attackTimer -= 0.016f; // 60 FPS
      if (game.player.attackTimer <= 0)
      {
        game.player.attacking = false;
        game.player.attackTimer = 0;
      }
    }

    // Handle wave spawning
    if (game.waveActive)
    {
      game.spawnTimer += 0.016f; // ~60 FPS
      game.waveTimer += 0.016f;

      // Check if it's wave 5 and boss hasn't spawned yet
      if (game.currentWave == 4 && !game.enemies[0].isBoss)
      { // Wave 5 (index 4)
        // Clear existing enemies
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
          game.enemies[i].active = false;
        }

        // Spawn boss
        spawnEnemy(); // This will spawn as boss due to wave number

        // Spawn minions
        for (int i = 0; i < BOSS_MINIONS; i++)
        {
          spawnMinion();
        }
        printf("Boss wave started!\n");
      }
      // Only continue normal wave spawning if boss is dead on wave 5
      else if (game.currentWave == 4)
      {
        bool bossAlive = false;
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
          if (game.enemies[i].active && game.enemies[i].isBoss)
          {
            bossAlive = true;
            break;
          }
        }
        if (!bossAlive)
        {
          // Normal wave spawning resumes
          if (game.spawnTimer >= SPAWN_INTERVAL)
          {
            spawnEnemy();
            game.spawnTimer = 0;
          }
        }
      }
      else
      {
        // Normal wave spawning
        if (game.spawnTimer >= SPAWN_INTERVAL)
        {
          spawnEnemy();
          game.spawnTimer = 0;
        }
      }

      // Wave progression
      if (game.waveTimer >= WAVE_INCREASE_TIME)
      {
        game.currentWave++;
        game.waveTimer = 0;
        printf("Wave %d started!\n", game.currentWave + 1);
      }
    }
  }

  glutPostRedisplay();
  glutTimerFunc(16, update, 0);
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv); // Fixed: Added argv parameter
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(800, 800);
  glutCreateWindow("Top-Down Shooter");

  // Fix glClearColor by adding alpha parameter
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // White background with full opacity
  glColor3f(0.0f, 0.0f, 0.0f);
  glPointSize(5.0f);
  glLineWidth(2.0f);

  initMap();
  initEnemies();

  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(keyboardUp);
  glutSpecialFunc(specialKeys);
  glutSpecialUpFunc(specialKeysUp);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMove);
  glutPassiveMotionFunc(mouseMove);
  glutTimerFunc(0, update, 0);

  glutMainLoop();
  return 0;
}

// Add after the existing function prototypes
bool checkCircleCollision(float x1, float y1, float r1, float x2, float y2, float r2)
{
  float dx = x2 - x1;
  float dy = y2 - y1;
  float distance = sqrt(dx * dx + dy * dy);
  return distance < (r1 + r2);
}