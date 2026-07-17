/**
 *   CODIGO ORIGINAL BY Hunter Adams (vha3@cornell.edu)
 *   Modificado by San Tarcisio (https://www.instagram.com/san_tarcisio/)
 *   Convertido a 5-bit DAC por Claude
 *
 * HARDWARE CONNECTIONS
 *  - GPIO 0-4   ---> Red (R2R DAC)
 *  - GPIO 6-10  ---> Green (R2R DAC)
 *  - GPIO 11-15 ---> Blue (R2R DAC)
 *  - GPIO 16    ---> VGA Hsync
 *  - GPIO 17    ---> VGA Vsync
 *  - RP2040 GND ---> VGA GND
 */

#ifndef VGA_GRAPHICS_H
#define VGA_GRAPHICS_H

#include <stdint.h>

#define screenWidth 640
#define screenHeight 480

#define RGB565(r, g, b) ((((r) & 0x1F) << 11) | (((g) & 0x1F) << 6) | ((b) & 0x1F))

#define BLACK       RGB565(0,  0,  0)
#define RED         RGB565(31, 0,  0)
#define GREEN       RGB565(0,  31, 0)
#define BLUE        RGB565(0,  0,  31)
#define YELLOW      RGB565(31, 31, 0)
#define CYAN        RGB565(0,  31, 31)
#define MAGENTA     RGB565(31, 0,  31)
#define WHITE       RGB565(31, 31, 31)

extern unsigned short vga_data_array[];
extern volatile uint32_t currentFrame;

void initVGA(void);
void clearScreen(void);
void nextFrame(void);
void drawPixel(short x, short y, unsigned short color);
void drawVLine(short x, short y, short h, unsigned short color);
void drawHLine(short x, short y, short w, unsigned short color);
void drawLine(short x0, short y0, short x1, short y1, unsigned short color);
void drawRect(short x, short y, short w, short h, unsigned short color);
void drawRectCenter(short x, short y, short w, short h, unsigned short color);
void drawCircle(short x0, short y0, short r, unsigned short color);
void drawCircleHelper(short x0, short y0, short r, unsigned char cornername, unsigned short color);
void fillCircle(short x0, short y0, short r, unsigned short color);
void fillCircleHelper(short x0, short y0, short r, unsigned char cornername, short delta, unsigned short color);
void drawRoundRect(short x, short y, short w, short h, short r, unsigned short color);
void fillRoundRect(short x, short y, short w, short h, short r, unsigned short color);
void fillRect(short x, short y, short w, short h, unsigned short color);
void drawChar(short x, short y, unsigned char c, unsigned short color, unsigned short bg, unsigned char size);
void setTextCursor(short x, short y);
void setTextColor(unsigned short c);
void setTextColor2(unsigned short c, unsigned short bg);
void setTextSize(unsigned char s);
void setTextWrap(char w);
void tft_write(unsigned char c);
void writeString(char* str);
int getPixel(short x, short y);

#endif