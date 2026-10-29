# Hardware Wiring Diagram (`pico-3d-mouse`)

## Overview

The Raspberry Pi Pico (RP2040, USB-C) connects to the host computer over USB-C and reads three potentiometer wipers tied across the Pico's analog supply (`3V3(OUT)`, Pin 36) and analog ground (`AGND`, Pin 33).

```mermaid
flowchart LR
    Host["Host Computer"] <==>|"USB-C (Power + USB-CDC / HID)"| USB["USB-C Port"]

    subgraph Pico["Raspberry Pi Pico (RP2040)"]
        USB
        Pin36["Pin 36: 3V3(OUT) (+3.3V)"]
        Pin34["Pin 34: GP28 / ADC2"]
        Pin33["Pin 33: AGND (Analog GND)"]
        Pin32["Pin 32: GP27 / ADC1"]
        Pin31["Pin 31: GP26 / ADC0"]
        LED["GP25: Onboard Status LED"]
    end

    subgraph Pots["3-Axis Joystick / Potentiometer Array"]
        PotFB["Forward / Backward Potentiometer"]
        PotLR["Left / Right Potentiometer"]
        PotRot["Rotation (Twist) Potentiometer"]
    end

    Pin36 -->|"+3.3V Rail"| PotFB
    Pin36 -->|"+3.3V Rail"| PotLR
    Pin36 -->|"+3.3V Rail"| PotRot

    PotFB -->|"AGND Rail"| Pin33
    PotLR -->|"AGND Rail"| Pin33
    PotRot -->|"AGND Rail"| Pin33

    PotFB ===>|"Wiper (Fwd/Back)"| Pin31
    PotLR ===>|"Wiper (Left/Right)"| Pin32
    PotRot ===>|"Wiper (Rotation)"| Pin34
```

## Schematic & Pin Connections

```text
                      ┌─────────────────────────────────────┐
                      │         HOST COMPUTER (PC)          │
                      └──────────────────┬──────────────────┘
                                         │
                                    USB-C Cable
                                         │
                    ┌────────────────────┴────────────────────┐
                    │               [ USB-C ]                 │
                    │                                         │
                    │         Raspberry Pi Pico (RP2040)      │
                    │                                         │
                    │                           Pin 36        │
                    │                         3V3(OUT) ───────┼───────┬───────────────┬───────────────┐
                    │                                         │       │               │               │
                    │                           Pin 34        │      ┌┴┐             ┌┴┐             ┌┴┐
                    │                      GP28 / ADC2 <──────┼──────┤ │<─ Wiper     │ │             │ │
                    │                                         │      │ │  ROTATION   │ │  LEFT/RIGHT │ │  FWD/BACK
                    │                           Pin 33        │      └┬┘  (Twist)    └┬┘             └┬┘
                    │                             AGND ───────┼───┬───┴───────────────┴───────────────┘
                    │                                         │   │                   ▲               ▲
                    │                           Pin 32        │   │                   │ Wiper         │ Wiper
                    │                      GP27 / ADC1 <──────┼───┼───────────────────┘               │
                    │                                         │   │                                   │
                    │                           Pin 31        │   │                                   │
                    │                      GP26 / ADC0 <──────┼───┼───────────────────────────────────┘
                    │                                         │   │
                    │  [GP25 Status LED]                      │  ─┴─ AGND
                    └─────────────────────────────────────────┘
```

## Pin Mapping Reference

| Axis / Signal | Potentiometer Terminal | RP2040 GPIO | ADC Channel | Physical Pin |
| :--- | :--- | :--- | :---: | :---: |
| **Forward / Backward** | Wiper | `GP26` | `ADC0` | **Pin 31** |
| **Left / Right** | Wiper | `GP27` | `ADC1` | **Pin 32** |
| **Rotation (Twist)** | Wiper | `GP28` | `ADC2` | **Pin 34** |
| **Analog +3.3 V Supply** | High end of all 3 pots | `3V3(OUT)` | — | **Pin 36** |
| **Analog Ground** | Low end of all 3 pots | `AGND` | — | **Pin 33** |
| **Status Heartbeat LED** | Onboard LED | `GP25` | — | Onboard |

![RP2040 Pinout](pinout.png)
