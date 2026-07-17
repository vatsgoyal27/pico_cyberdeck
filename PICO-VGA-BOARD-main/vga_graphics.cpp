/*
  https://vanhunteradams.com/Pico/VGA/VGA.html modified
*/

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
// Our assembled programs:
// Each gets the name <pio_filename.pio.h>
#include "hsync.pio.h"
#include "vsync.pio.h"
#include "rgb.pio.h"
// Header file
#include "vga_graphics.h"
// Font file
#include "glcdfont.h"

// VGA timing constants
#define H_ACTIVE 655    // (active + frontporch - 1) - one cycle delay for mov
#define V_ACTIVE 479    // (active - 1)
#define RGB_ACTIVE 319  // (horizontal active)/2 - 1

// Length of the pixel array, and number of DMA transfers
const unsigned int pixels = screenWidth * screenHeight;
#define TXCOUNT pixels              // Total pixels (no more division by 2)
#define DMATXCOUNT screenWidth      // Cantidad de transferencias DMA = Largo de la scanline
const int QVGALastLine = 240;       // La cantidad de lineas de los gráficos

// Pixel color array that is DMA's to the PIO machines and
// a pointer to the ADDRESS of this color array.
// Note that this array is automatically initialized to all 0's (black)
volatile unsigned short* address_pointer_array = &vga_data_array[0];
unsigned short vga_data_array[TXCOUNT];

// For drawLine
#define swap(a, b) \
  { \
    short t = a; \
    a = b; \
    b = t; \
  }

// For writing text
#define tabspace 4  // number of spaces for a tab

// For accessing the font library
#define pgm_read_byte(addr) (*(const unsigned char*)(addr))

// For drawing characters
unsigned short cursor_y, cursor_x, textsize;
unsigned short textcolor, textbgcolor;
char wrap;

// DMA channels - 0 sends color data, 1 reconfigures and restarts 0
int rgb_dma_pio = dma_claim_unused_channel(true);
int rgb_dma_cb = dma_claim_unused_channel(true);

volatile uint32_t currentFrame;  // frame counter

volatile int currentScanLine;  // current processed scan line 0... (next displayed scan line)

// QVGA DMA handler - called on end of every scanline
void __not_in_flash_func(QVgaLine)() {
  // Clear the interrupt request for DMA control channel
  dma_hw->ints0 = (1u << rgb_dma_pio);
  // update DMA control channel and run it

  // increment scanline (1..)
  currentScanLine++;             // new current scanline
  if (currentScanLine >= 480) {  // last scanline?
    currentFrame++;              // increment frame counter
    currentScanLine = 0;         // restart scanline
  }

  address_pointer_array =
    &vga_data_array[DMATXCOUNT * currentScanLine];
}

void initVGA() {

  set_sys_clock_khz(126000, true);
  // Choose which PIO instance to use (there are two instances, each with 4 state machines)
  PIO pio = pio0;

  // Our assembled program needs to be loaded into this PIO's instruction
  // memory. This SDK function will find a location (offset) in the
  // instruction memory where there is enough space for our program. We need
  // to remember these locations!
  //
  // We only have 32 instructions to spend! If the PIO programs contain more than
  // 32 instructions, then an error message will get thrown at these lines of code.
  //
  // The program name comes from the .program part of the pio file
  // and is of the form <program name_program>
  uint hsync_offset = pio_add_program(pio, &hsync_program);
  uint vsync_offset = pio_add_program(pio, &vsync_program);
  uint rgb_offset = pio_add_program(pio, &rgb_program);

  // Manually select a few state machines from pio instance pio0.
  uint hsync_sm = 0;
  uint vsync_sm = 1;
  uint rgb_sm = 2;

  // Call the initialization functions that are defined within each PIO file.
  // Why not create these programs here? By putting the initialization function in
  // the pio file, then all information about how to use/setup that state machine
  // is consolidated in one place. Here in the C, we then just import and use it.
  hsync_program_init(pio, hsync_sm, hsync_offset, 16);  // HSYNC on GPIO 16
  vsync_program_init(pio, vsync_sm, vsync_offset, 17);  // VSYNC on GPIO 17
  rgb_program_init(pio, rgb_sm, rgb_offset, 0);         // RGB DAC on GPIO 0-4, 6-10, 11-15

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // ============================== PIO DMA Channels =================================================
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  // Channel Zero (sends color data to PIO VGA machine)
  dma_channel_config c0 = dma_channel_get_default_config(rgb_dma_pio);  // default configs
  channel_config_set_transfer_data_size(&c0, DMA_SIZE_16);              // 16-bit txfers (changed from 8-bit)
  channel_config_set_read_increment(&c0, true);                         // yes read incrementing
  channel_config_set_write_increment(&c0, false);                       // no write incrementing
  channel_config_set_dreq(&c0, DREQ_PIO0_TX2);                          // DREQ_PIO0_TX2 pacing (FIFO)
  channel_config_set_chain_to(&c0, rgb_dma_cb);                         // chain to other channel

  dma_channel_configure(
    rgb_dma_pio,        // Channel to be configured
    &c0,                // The configuration we just created
    &pio->txf[rgb_sm],  // write address (RGB PIO TX FIFO)
    &vga_data_array,    // The initial read address (pixel color array)
    DMATXCOUNT,         // Number of transfers
    false               // Don't start immediately.
  );

  // Channel One (reconfigures the first channel)
  dma_channel_config c1 = dma_channel_get_default_config(rgb_dma_cb);  // default configs
  channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);             // 32-bit txfers
  channel_config_set_read_increment(&c1, false);                       // no read incrementing
  channel_config_set_write_increment(&c1, false);                      // no write incrementing
  channel_config_set_chain_to(&c1, rgb_dma_pio);                       // chain to other channel

  dma_channel_configure(
    rgb_dma_cb,                          // Channel to be configured
    &c1,                                 // The configuration we just created
    &dma_hw->ch[rgb_dma_pio].read_addr,  // Write address (channel 0 read address)
    &address_pointer_array,              // Read address (POINTER TO AN ADDRESS)
    1,                                   // Number of transfers, in this case each is 4 byte
    false                                // Don't start immediately.
  );

  // enable DMA channel IRQ0
  dma_channel_set_irq0_enabled(rgb_dma_pio, true);
  // set DMA IRQ handler
  irq_set_exclusive_handler(DMA_IRQ_0, QVgaLine);
  irq_set_enabled(DMA_IRQ_0, true);
  // set highest IRQ priority
  irq_set_priority(DMA_IRQ_0, 0);
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  // Initialize PIO state machine counters. This passes the information to the state machines
  // that they retrieve in the first 'pull' instructions, before the .wrap_target directive
  // in the assembly. Each uses these values to initialize some counting registers.
  pio_sm_put_blocking(pio, hsync_sm, H_ACTIVE);
  pio_sm_put_blocking(pio, vsync_sm, V_ACTIVE);
  pio_sm_put_blocking(pio, rgb_sm, RGB_ACTIVE);

  // Start the two pio machine IN SYNC
  // Note that the RGB state machine is running at full speed,
  // so synchronization doesn't matter for that one. But, we'll
  // start them all simultaneously anyway.
  pio_enable_sm_mask_in_sync(pio, ((1u << hsync_sm) | (1u << vsync_sm) | (1u << rgb_sm)));

  // Start DMA channel 0. Once started, the contents of the pixel color array
  // will be continously DMA's to the PIO machines that are driving the screen.
  // To change the contents of the screen, we need only change the contents
  // of that array.
  dma_start_channel_mask((1u << rgb_dma_pio));
}

// A function for drawing a pixel with a specified color.
// Note that because information is passed to the PIO state machines through
// a DMA channel, we only need to modify the contents of the array and the
// pixels will be automatically updated on the screen.
void drawPixel(short x, short y, unsigned short color) {
  if (x < 0 || y < 0 || x >= screenWidth || y >= screenHeight) return;

  int pixel = (screenWidth * y) + x;
  vga_data_array[pixel] = color;  // Direct 16-bit write
}

void clearScreen() {
  for (int i = 0; i < TXCOUNT; i++) {
    vga_data_array[i] = 0;
  }
}

void nextFrame() {
  // no-op (single buffer mode)
}

void drawVLine(short x, short y, short h, unsigned short color) {
  for (short i = y; i < (y + h); i++) {
    drawPixel(x, i, color);
  }
}

void drawHLine(short x, short y, short w, unsigned short color) {
  for (short i = x; i < (x + w); i++) {
    drawPixel(i, y, color);
  }
}

// Bresenham's algorithm - thx wikipedia and thx Bruce!
void drawLine(short x0, short y0, short x1, short y1, unsigned short color) {
  short steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  short dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  short err = dx / 2;
  short ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

// Draw a rectangle
void drawRect(short x, short y, short w, short h, unsigned short color) {
  drawHLine(x, y, w, color);
  drawHLine(x, y + h - 1, w, color);
  drawVLine(x, y, h, color);
  drawVLine(x + w - 1, y, h, color);
}

void drawRectCenter(short x, short y, short w, short h, unsigned short color) {
  drawRect(x - w / 2, y - h / 2, w, h, color);
}

void drawCircle(short x0, short y0, short r, unsigned short color) {
  short f = 1 - r;
  short ddF_x = 1;
  short ddF_y = -2 * r;
  short x = 0;
  short y = r;

  drawPixel(x0, y0 + r, color);
  drawPixel(x0, y0 - r, color);
  drawPixel(x0 + r, y0, color);
  drawPixel(x0 - r, y0, color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
  }
}

void drawCircleHelper(short x0, short y0, short r, unsigned char cornername, unsigned short color) {
  short f = 1 - r;
  short ddF_x = 1;
  short ddF_y = -2 * r;
  short x = 0;
  short y = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4) {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    }
    if (cornername & 0x2) {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

void fillCircle(short x0, short y0, short r, unsigned short color) {
  drawVLine(x0, y0 - r, 2 * r + 1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}

void fillCircleHelper(short x0, short y0, short r, unsigned char cornername, short delta, unsigned short color) {
  short f = 1 - r;
  short ddF_x = 1;
  short ddF_y = -2 * r;
  short x = 0;
  short y = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    if (cornername & 0x1) {
      drawVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
      drawVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
    }
    if (cornername & 0x2) {
      drawVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
      drawVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
    }
  }
}

// Draw a rounded rectangle
void drawRoundRect(short x, short y, short w, short h, short r, unsigned short color) {
  drawHLine(x + r, y, w - 2 * r, color);
  drawHLine(x + r, y + h - 1, w - 2 * r, color);
  drawVLine(x, y + r, h - 2 * r, color);
  drawVLine(x + w - 1, y + r, h - 2 * r, color);
  drawCircleHelper(x + r, y + r, r, 1, color);
  drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
  drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
  drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
}

// Fill a rounded rectangle
void fillRoundRect(short x, short y, short w, short h, short r, unsigned short color) {
  fillRect(x + r, y, w - 2 * r, h, color);
  fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
  fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

// fill a rectangle
void fillRect(short x, short y, short w, short h, unsigned short color) {
  for (int i = x; i < (x + w); i++) {
    for (int j = y; j < (y + h); j++) {
      drawPixel(i, j, color);
    }
  }
}

// Draw a character
void drawChar(short x, short y, unsigned char c, unsigned short color, unsigned short bg, unsigned char size) {
  char i, j;
  if ((x >= screenWidth) ||
      (y >= screenHeight) ||
      ((x + 6 * size - 1) < 0) ||
      ((y + 8 * size - 1) < 0))
    return;

  for (i = 0; i < 6; i++) {
    unsigned char line;
    if (i == 5)
      line = 0x0;
    else
      line = pgm_read_byte(font + (c * 5) + i);
    for (j = 0; j < 8; j++) {
      if (line & 0x1) {
        if (size == 1)
          drawPixel(x + i, y + j, color);
        else {
          fillRect(x + (i * size), y + (j * size), size, size, color);
        }
      } else if (bg != color) {
        if (size == 1)
          drawPixel(x + i, y + j, bg);
        else {
          fillRect(x + i * size, y + j * size, size, size, bg);
        }
      }
      line >>= 1;
    }
  }
}

void setTextCursor(short x, short y) {
  cursor_x = x;
  cursor_y = y;
}

void setTextSize(unsigned char s) {
  textsize = (s > 0) ? s : 1;
}

void setTextColor(unsigned short c) {
  textcolor = textbgcolor = c;
}

void setTextColor2(unsigned short c, unsigned short b) {
  textcolor = c;
  textbgcolor = b;
}

void setTextWrap(char w) {
  wrap = w;
}

void tft_write(unsigned char c) {
  if (c == '\n') {
    cursor_y += textsize * 8;
    cursor_x = 0;
  } else if (c == '\r') {
    // skip em
  } else if (c == '\t') {
    int new_x = cursor_x + tabspace;
    if (new_x < screenWidth) {
      cursor_x = new_x;
    }
  } else {
    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
    cursor_x += textsize * 6;
    if (wrap && (cursor_x > (screenWidth - textsize * 6))) {
      cursor_y += textsize * 8;
      cursor_x = 0;
    }
  }
}

void writeString(char* str) {
  while (*str) {
    tft_write(*str++);
  }
}

// Check if alive
int getPixel(short x, short y) {
  int pixel = (screenWidth * y) + x;
  return (vga_data_array[pixel] & 1);
}