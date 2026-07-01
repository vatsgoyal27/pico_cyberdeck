# Modular Pico Cyberdeck

A custom cyberdeck built entirely around Raspberry Pi Pico 2 W boards instead of a traditional single-board computer (e.g. Raspberry Pi 4/5). Rather than one processor running the whole device, the deck is split into independent modules — each powered by its own Pico 2 W — handling a specific subsystem: display/audio, UI, input, and network scanning.

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
| **UI Controller** | Renders menus/interface, coordinates modules | 🚧 In progress |
| **Input Handler** | Keyboard/button/joystick input | 🚧 In progress |
| **Network Scanner** | Wi-Fi/network scanning and analysis | 🚧 In progress |

Inter-module communication method (e.g. I2C/SPI/UART bus, or Wi-Fi between Pico 2 W boards)

---

## Sub-Project: VGA + Audio Driver ✅

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

## Getting Started

### Hardware

1. Open each module's KiCad project.
2. Export Gerbers/fabrication files per module as needed.
3. Reference the shared design notes (trace impedance, connector pinouts) across modules for consistency.

### Firmware

Each module runs independently on its own Pico 2 W. Firmware approach (MicroPython / CircuitPython / C SDK) — *to be finalized per module.*

## Status

🚧 Work in progress — VGA + audio module complete; UI, input, and network scanning modules in development.
