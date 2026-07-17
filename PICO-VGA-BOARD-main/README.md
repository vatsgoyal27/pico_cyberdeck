# PICO-VGA-BOARD


This project is a modified version of the original VGA graphics library by Pancrea85, adapted to generate a true 640×480 VGA signal from the Raspberry Pi Pico using the Arduino IDE. 

https://github.com/Pancra85/VGA_graphics //ORIGINAL LIBRARY

Alongside these improvements, I designed a custom Pico VGA Board—a dedicated PCB that integrates a DB15 VGA connector directly with the Pico for a clean and reliable video output setup.

Using this setup, I’ve built several demo projects, including Conway’s Game of Life, a Snake game, and a simple Hello World renderer, all included within this repository as examples of what the library can achieve.

---------------------------------------------------
CHANGES I MADE
---------------------------------------------------

1. Switched from QVGA-style assumptions to real VGA

// Screen width/height in ORIGINAL vga_graphics.h
#define screenWidth 640 //in pixels
#define screenHeight 240 //in pixels


// Screen width/height in my Version
#define screenWidth 640 //in pixels
#define screenHeight 480 //in pixels



2. In vga_graphics.cpp

a) We removed scanline doubling

//Original CRT/QVGA behavior
address_pointer_array =
  &vga_data_array[DMATXCOUNT * ((currentScanLine + 1) >> 1)];

//Our True VGA Change
address_pointer_array =
  &vga_data_array[DMATXCOUNT * ((currentScanLine + 1) >> 1)];

That >> 1 was duplicating every scanline, and Modern LCDs reject that,
So Removing it makes one framebuffer line = one VGA scanline.

b) Disable double buffering

unsigned char vga_data_array_next[TXCOUNT];

We simply removed the whole second buffer line; this will make drawings write directly to vga_data_array and turn nextFrame() into a no-op.


640×480 + color + double buffer does not fit in RP2040 RAM.
The single buffer is stable.

3. In vga_graphics.cpp, inside initVGA() we fix system clock. 

Inside initVGA() at the very top, we added -

#include "hardware/clocks.h"

Then we add the line below to it.

set_sys_clock_khz(126000, true);

Arduino’s default Pico clock causes pixel timing drift.
For VGA, timing must be exact, or modern monitors reject the signal.

4. VGA timing constants Update.

#define H_ACTIVE   799
#define V_ACTIVE   524
#define RGB_ACTIVE 639

This describes 640×480 @ 60 Hz, which modern LCDs accept.

-------------------------------------------
INSTALLATION PROCESS
-------------------------------------------

This library is distributed as a ZIP file.

Download the ZIP file from this repository.
Open the Arduino IDE.
Go to Sketch → Include Library → Add .ZIP Library…
Select the downloaded ZIP file.

Once added, you can access the included demo projects by navigating to:
File → Examples → PICO VGA 

You can now use this library like any other standard Arduino library in your projects.



