#include "vga_graphics.h"

/* ── Screen ───────────────── */
#define SW 640
#define SH 480

/* ── Grid ───────────────── */
#define CELL 8
#define GRID_W (SW / CELL)
#define GRID_H (SH / CELL)

/* ── Buffers ───────────── */
bool grid[GRID_W][GRID_H];
bool nextGrid[GRID_W][GRID_H];

/* ── Draw cell ─────────── */
void drawCell(int x, int y, bool alive) {
  fillRect(
    x * CELL,
    y * CELL,
    CELL - 1,
    CELL - 1,
    alive ? YELLOW : BLACK
  );
}

/* ── Random init ───────── */
void randomizeGrid() {
  for (int x = 0; x < GRID_W; x++) {
    for (int y = 0; y < GRID_H; y++) {
      grid[x][y] = random(0, 2);
      drawCell(x, y, grid[x][y]);
    }
  }
}

/* ── Count neighbors ───── */
int countNeighbors(int x, int y) {
  int count = 0;

  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dy <= 1; dy++) {
      if (dx == 0 && dy == 0) continue;

      int nx = x + dx;
      int ny = y + dy;

      // wrap around edges
      if (nx < 0) nx = GRID_W - 1;
      if (nx >= GRID_W) nx = 0;
      if (ny < 0) ny = GRID_H - 1;
      if (ny >= GRID_H) ny = 0;

      if (grid[nx][ny]) count++;
    }
  }

  return count;
}

/* ── Update simulation ─── */
void updateGrid() {

  for (int x = 0; x < GRID_W; x++) {
    for (int y = 0; y < GRID_H; y++) {

      int neighbors = countNeighbors(x, y);

      if (grid[x][y]) {
        // alive
        nextGrid[x][y] = (neighbors == 2 || neighbors == 3);
      } else {
        // dead
        nextGrid[x][y] = (neighbors == 3);
      }
    }
  }

  // apply + redraw only changes
  for (int x = 0; x < GRID_W; x++) {
    for (int y = 0; y < GRID_H; y++) {

      if (grid[x][y] != nextGrid[x][y]) {
        drawCell(x, y, nextGrid[x][y]);
      }

      grid[x][y] = nextGrid[x][y];
    }
  }
}

/* ── Setup ─────────────── */
void setup() {
  initVGA();
  clearScreen();

  randomSeed(analogRead(26)); // Pico randomness

  randomizeGrid();
}

/* ── Loop ─────────────── */
void loop() {
  updateGrid();
  delay(80);  // speed control
}