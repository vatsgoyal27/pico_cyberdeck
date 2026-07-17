#include "vga_graphics.h"
#include <string.h>

/* ── Screen ───────────────── */
#define SW 640
#define SH 480

/* ── Text scale ──────────── */
#define SCALE 4

/* ── Centered print ───────── */
void printCentered(const char* text, int y, unsigned short color) {
  int len = strlen(text);
  int textWidth = len * 6 * SCALE;
  int x = (SW - textWidth) / 2;

  for (int i = 0; i < len; i++) {
    drawChar(x + i * 6 * SCALE, y, text[i], color, BLACK, SCALE);
  }
}

/* ── CRT Scanlines ───────── */
void drawScanlines() {
  Serial.println("Drawing scanlines...");
  for (int y = 0; y < SH; y += 2) {
    drawHLine(0, y, SW, BLACK);
  }
  Serial.println("Scanlines done");
}

/* ── Setup ─────────────── */
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== VGA Hello World Demo ===");
  Serial.println("Initializing VGA...");
  
  initVGA();
  
  Serial.println("VGA initialized!");
  Serial.println("Clearing screen...");
  clearScreen();
  
  Serial.println("Drawing text...");

  int lineHeight = 8 * SCALE;
  int totalHeight = lineHeight * 3 + 20;
  int startY = (SH - totalHeight) / 2;

  Serial.print("Line height: ");
  Serial.println(lineHeight);
  Serial.print("Start Y: ");
  Serial.println(startY);

  // Line 1 (RED)
  Serial.println("Drawing line 1 (RED)...");
  printCentered("HELLO WORLD 1", startY, RED);
  Serial.println("Line 1 done");

  // Line 2 (GREEN)
  Serial.println("Drawing line 2 (GREEN)...");
  printCentered("HELLO WORLD 2", startY + lineHeight + 10, GREEN);
  Serial.println("Line 2 done");

  // Line 3 (BLUE)
  Serial.println("Drawing line 3 (BLUE)...");
  printCentered("HELLO WORLD 3", startY + (lineHeight + 10) * 2, BLUE);
  Serial.println("Line 3 done");

  // CRT overlay
  Serial.println("Adding scanlines...");
  drawScanlines();

  Serial.println("=== Setup Complete ===");
  Serial.print("Current frame: ");
  Serial.println(currentFrame);
}

/* ── Loop ─────────────── */
void loop() {
  delay(1000);
  
  Serial.print("Frame: ");
  Serial.println(currentFrame);
}