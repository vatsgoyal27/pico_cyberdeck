# Modular Pico Cyberdeck
!(image.png)
A custom cyberdeck built entirely around Raspberry Pi Pico 2 W boards instead of a traditional single-board computer (e.g. Raspberry Pi 4/5). Rather than one processor running the whole device, the deck is split into independent modules — each powered by its own Pico 2 W — handling a specific subsystem: display/audio, input (keyboard + joystick), Spotify display, UI, and network scanning.

## Why This Is Different

Most cyberdecks are built around a full Linux-capable SBC (Raspberry Pi, Odroid, etc.) running a conventional OS with a desktop environment, shell, and multitasking kernel. This project takes a different approach:

- **No traditional OS** — the RP2350-based Pico 2 W doesn't run Linux, Android, or any general-purpose OS. Each module runs bare-metal firmware or a lightweight RTOS (e.g. MicroPython, CircuitPython, or C/C++ via the Pico SDK).
- **Distributed, not centralized** — instead of one OS managing everything, functionality is distributed across multiple independent microcontrollers, each with a single job.
- **No shell, no window manager** — "multitasking" here means multiple physical boards running in parallel, not OS-level process scheduling.

This makes the build process, inter-module communication, and system architecture fundamentally different from a typical Pi-based cyberdeck, closer to a distributed embedded system than a general-purpose computer.

## System Architecture

| Module | Role | Status |
|---|---|---|
| **Display + Audio Driver** | Drives VGA output and mono audio | ✅ Complete |
| **Input Handler — Keyboard + Joystick** | Custom mechanical keyboard with integrated analog joystick | ✅ Complete |
| **Spotify Pico Display** | LCD + TFT display module with joystick control | ✅ Complete |
| **UI Controller** | Renders menus/interface, coordinates modules | 🚧 In progress |
| **Network Scanner** | Wi-Fi/network scanning and analysis | 🚧 In progress |

Inter-module communication method (e.g. I2C/SPI/UART bus, or Wi-Fi between Pico 2 W boards) — *TBD, document once finalized.*

---

## Sub-Project: Display + Audio Driver ✅

The first completed module. A Raspberry Pi Pico generates analog VGA video output and drives a mono audio amplifier.

### Features

- **VGA output** via DE-15 connector, using a resistor-ladder DAC on GPIO pins for RGB signal generation, plus dedicated HSYNC/VSYNC lines
- **Mono audio output** via a MAX98357A I2S Class D amplifier module
- **MicroSD card slot** for storing media/assets
- Designed for 75Ω characteristic impedance signal traces (per design notes: 0.3mm traces for 4-layer, 1.4mm for 2-layer boards)

### Hardware

| Component | Details |
|---|---|
| MCU | Raspberry Pi Pico (2 W) |
| Video Output | DE-15 (VGA) connector, resistor DAC for RGB |
| Audio | MAX98357A I2S mono Class D amplifier |
| Storage | MicroSD card slot |
| Mounting | 4x mounting holes |

### Status

Schematic and PCB design complete (KiCad, rev. 2026-06-01). Firmware for video timing generation and audio playback: *document current state here.*

---

## Sub-Project: Input Handler — Keyboard + Joystick ✅

A custom mechanical keyboard with an integrated analog joystick, serving as the cyberdeck's primary input module. Powered by its own Raspberry Pi Pico (RP2040).

### Overview

This module is a fully custom keyboard PCB featuring an analog joystick for cursor/pointer control alongside the key matrix. It's built around a Raspberry Pi Pico, with onboard Li-Po battery charging for wireless/portable use.

### Features

- **~44 mechanical key switches** with per-switch diodes for matrix scanning
- **Integrated analog joystick** for mouse/pointer input
- **Raspberry Pi Pico (RP2040)** as the onboard microcontroller
- **USB-C charging** via TP4056 charge controller for a Li-Po battery
- **Mounting holes** for case/plate assembly
- Designed entirely in **KiCad** (schematic + PCB layout)

### Hardware

| Component | Details |
|---|---|
| MCU | Raspberry Pi Pico |
| Switches | ~44x mechanical key switches (SW1–SW44) |
| Diodes | Per-key diodes (1N4148 or similar) for matrix scanning |
| Charging | TP4056 Micro-USB/USB-C Li-Po charger |
| Input | Analog joystick module |
| Connectors | 2x05 and 2x10 pin sockets (peripheral connections) |
| Battery | Li-Po (connector on board) |

### Assembly

1. Solder switches and diodes to the board.
2. Mount the Raspberry Pi Pico and TP4056 charging module.
3. Connect the joystick module to its designated header.
4. Flash firmware and test the key matrix + joystick input.

### Status

Hardware and firmware complete.

---

## Sub-Project: Spotify Pico Display ✅

A display module combining a character LCD and a TFT screen with joystick input, driven by a Raspberry Pi Pico 2 W. Displays Spotify playback info (album art, track data) on the cyberdeck.

### Pinout

**MCU:** Raspberry Pi Pico 2 W

**LCD (LiquidCrystal — 4-bit mode)**

| Function | GPIO |
|----------|------|
| RS       | 2    |
| EN       | 3    |
| D4       | 4    |
| D5       | 5    |
| D6       | 6    |
| D7       | 7    |

**Joystick (Analog)**

| Function | GPIO | Notes        |
|----------|------|--------------|
| X        | 26   | ADC0         |
| Y        | 27   | ADC1         |
| SW       | 22   | INPUT_PULLUP |

**TFT Display (ST7735 — SPI)**

| Function | GPIO |
|----------|------|
| CS       | 17   |
| DC       | 20   |
| RST      | 21   |
| SCK      | 18   |
| MOSI     | 19   |
| MISO     | 16   |

### Notes

The temporary GIF display is implemented using [image_to_c](https://github.com/bitbank2/image_to_c).

![Wiring diagram](spotify_api_display_controller/wiring.jpeg)

### Status

Hardware and firmware complete.

---

## Getting Started

### Hardware

1. Open each module's KiCad project under its respective folder.
2. Export Gerbers/fabrication files per module as needed.
3. Reference the shared design notes (trace impedance, connector pinouts) across modules for consistency.

### Firmware

Each module runs independently on its own Pico / Pico 2 W. Firmware approach (MicroPython / CircuitPython / C SDK) — to be finalized per module.

## Status

🚧 Work in progress — Display + Audio, Input Handler, and Spotify Pico Display modules complete; UI and Network Scanner modules in development.
