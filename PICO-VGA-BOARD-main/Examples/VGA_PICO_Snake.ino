#include "vga_graphics.h"

/* ── Screen ───────────────── */
#define SW 640
#define SH 480

/* ── Grid ───────────────── */
#define CELL 12
#define GRID_W (SW / CELL)
#define GRID_H (SH / CELL)

/* ── Snake ─────────────── */
struct Segment {
  int x;
  int y;
};

Segment snake[200];
int snakeLength = 5;

int dx = 1, dy = 0;

int foodX, foodY;
int prevTailX, prevTailY;

/* ── Random food ───────── */
void placeFood() {
  foodX = random(2, GRID_W - 2);
  foodY = random(2, GRID_H - 2);
}

/* ── Reset ─────────────── */
void resetGame() {
  clearScreen();

  snakeLength = 5;
  dx = 1; dy = 0;

  for (int i = 0; i < snakeLength; i++) {
    snake[i].x = GRID_W / 2 - i;
    snake[i].y = GRID_H / 2;
  }

  placeFood();
}

/* ── Random movement ───── */
void randomMove() {
  if (random(0, 10) < 3) {  // 30% chance

    int dir = random(0, 4);

    if (dir == 0 && dy == 0) { dx = 0; dy = -1; } // up
    if (dir == 1 && dy == 0) { dx = 0; dy = 1; }  // down
    if (dir == 2 && dx == 0) { dx = -1; dy = 0; } // left
    if (dir == 3 && dx == 0) { dx = 1; dy = 0; }  // right
  }
}

/* ── Collision ─────────── */
bool selfCollision() {
  for (int i = 1; i < snakeLength; i++) {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
      return true;
    }
  }
  return false;
}

/* ── Draw one cell ─────── */
void drawCell(int gx, int gy, char color) {
  fillRect(gx * CELL, gy * CELL, CELL - 1, CELL - 1, color);
}

/* ── Setup ─────────────── */
void setup() {
  initVGA();
  clearScreen();

  randomSeed(analogRead(26)); // Pico randomness

  resetGame();
}

/* ── Loop ─────────────── */
void loop() {

  // random AI movement
  randomMove();

  // store tail
  prevTailX = snake[snakeLength - 1].x;
  prevTailY = snake[snakeLength - 1].y;

  // move body
  for (int i = snakeLength - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }

  // move head
  snake[0].x += dx;
  snake[0].y += dy;

  // wrap screen
  if (snake[0].x >= GRID_W) snake[0].x = 0;
  if (snake[0].x < 0) snake[0].x = GRID_W - 1;
  if (snake[0].y >= GRID_H) snake[0].y = 0;
  if (snake[0].y < 0) snake[0].y = GRID_H - 1;

  // self collision
  if (selfCollision()) {
    delay(500);
    resetGame();
    return;
  }

  // food
  if (snake[0].x == foodX && snake[0].y == foodY) {
    snakeLength++;
    placeFood();
  }

  // erase tail
  drawCell(prevTailX, prevTailY, BLACK);

  // draw snake
  for (int i = 0; i < snakeLength; i++) {
    drawCell(snake[i].x, snake[i].y, GREEN);
  }

  // draw food
  drawCell(foodX, foodY, WHITE);

  delay(120);
}