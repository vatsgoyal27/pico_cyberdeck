#include "vga_graphics.h"
#include <string.h>

/* ── Screen ───────────────── */
#define SW 640
#define SH 480

/* ── Text scale ──────────── */
#define SCALE 4

/* ── Centered print ───────── */
void printCentered(const char* text, int y, char color) {
  int len = strlen(text);
  int textWidth = len * 6 * SCALE;
  int x = (SW - textWidth) / 2;

  for (int i = 0; i < len; i++) {
    drawChar(x + i * 6 * SCALE, y, text[i], color, BLACK, SCALE);
  }
}

/* ── CRT Scanlines ───────── */
void drawScanlines() {
  for (int y = 0; y < SH; y += 2) {
    drawHLine(0, y, SW, BLACK);  // dark line every other row
  }
}

/* ── Setup ─────────────── */
void setup() {
  initVGA();
  clearScreen();

  int lineHeight = 8 * SCALE;

  int totalHeight = lineHeight * 3 + 20; // spacing included
  int startY = (SH - totalHeight) / 2;

  // Line 1 (RED)
  printCentered("HELLO WORLD 1", startY, RED);

  // Line 2 (GREEN)
  printCentered("HELLO WORLD 2", startY + lineHeight + 10, GREEN);

  // Line 3 (BLUE)
  printCentered("HELLO WORLD 3", startY + (lineHeight + 10) * 2, BLUE);

  // CRT overlay
  drawScanlines();
}

/* ── Loop ─────────────── */
void loop() {
}
