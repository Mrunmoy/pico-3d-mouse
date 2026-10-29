# pico-3d-mouse

RP2040-based 3D mouse firmware using a 3-axis analog joystick / potentiometer array on the Raspberry Pi Pico.

![Raspberry Pi Pico Pinout](docs/pinout.png)

## Hardware Wiring

Each potentiometer has its outer terminals wired between the Pico's analog reference supply and analog ground, with the three wipers routed to the RP2040's three user ADC channels:

| Signal | Pico Pin | GPIO / Function | Notes |
| :--- | :---: | :--- | :--- |
| **Potentiometer +V** | Pin 36 | `3V3(OUT)` | +3.3 V supply rail |
| **Potentiometer GND** | Pin 33 | `AGND` | Analog ground reference |
| **Wiper 0 (Ch 0)** | Pin 31 | `GP26 / ADC0` | Analog channel 0 |
| **Wiper 1 (Ch 1)** | Pin 32 | `GP27 / ADC1` | Analog channel 1 |
| **Wiper 2 (Ch 2)** | Pin 34 | `GP28 / ADC2` | Analog channel 2 |
| **Status LED** | — | `GP25` | Onboard LED |

## Repository Layout

```text
.
├── docs/
│   └── pinout.png                      # RP2040 board pinout reference
├── firmware/
│   ├── CMakeLists.txt                  # Top-level C++17 Pico SDK build
│   ├── cmake/
│   │   └── firmware_app.cmake          # Source-scoped strict compiler warnings
│   ├── include/
│   │   ├── board/pins.hpp              # Authoritative pin & ADC channel map
│   │   └── drivers/pots.hpp            # 3-channel ADC potentiometer driver
│   ├── lib/
│   │   └── drivers_pots/               # Oversampled, mux-settled ADC reader
│   └── apps/
│       └── adc_bringup/                # USB-CDC 3-axis telemetry & calibration app
└── flake.nix                           # Reproducible Nix devShell (arm-none-eabi + pico-sdk 2.2.0)
```

## Building Firmware

Enter the Nix development shell (or run via `nix develop --command`):

```bash
nix develop
cmake -S firmware -B firmware/build -G Ninja
cmake --build firmware/build
```

## Flashing & Telemetry (`adc_bringup`)

Load onto the RP2040 over USB in BOOTSEL mode (or warm-reboot with `picotool`):

```bash
picotool load -f firmware/build/apps/adc_bringup/adc_bringup.uf2
picotool reboot
```

Monitor the 3 analog channels over USB-CDC (press `z` at any time to re-zero the resting center):

```bash
picocom -b 115200 /dev/ttyACM0
```
