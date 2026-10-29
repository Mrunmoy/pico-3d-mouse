# pico-3d-mouse

Self-contained RP2040 3D mouse firmware built with C++17, SOLID hardware abstractions, and full off-target TDD (GoogleTest + GMock).

## Hardware Wiring

The Raspberry Pi Pico (RP2040) connects to the host computer over **USB-C**. The three potentiometer outer terminals are tied in parallel across `3V3(OUT)` (Pin 36) and `AGND` (Pin 33), with their wipers routed to `ADC0`, `ADC1`, and `ADC2`:

| Axis / Signal | Potentiometer Terminal | RP2040 GPIO | ADC Channel | Physical Pin |
| :--- | :--- | :--- | :---: | :---: |
| **Forward / Backward** | Wiper | `GP26` | `ADC0` | **Pin 31** |
| **Left / Right** | Wiper | `GP27` | `ADC1` | **Pin 32** |
| **Rotation (Twist)** | Wiper | `GP28` | `ADC2` | **Pin 34** |
| **Potentiometer +V** | High side (all 3 pots) | `3V3(OUT)` | — | **Pin 36** |
| **Potentiometer GND** | Low side (all 3 pots) | `AGND` | — | **Pin 33** |
| **Status LED** | Onboard LED | `GP25` | — | Onboard |

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
    end

    subgraph Pots["3-Axis Joystick / Potentiometer Array"]
        PotFB["Forward / Backward Pot"]
        PotLR["Left / Right Pot"]
        PotRot["Rotation (Twist) Pot"]
    end

    Pin36 -->|"+3.3V"| PotFB & PotLR & PotRot
    PotFB & PotLR & PotRot -->|"AGND"| Pin33

    PotFB ===>|"Wiper"| Pin31
    PotLR ===>|"Wiper"| Pin32
    PotRot ===>|"Wiper"| Pin34
```

See [docs/WIRING.md](docs/WIRING.md) and [docs/pinout.png](docs/pinout.png) for the full schematic and board pinout.

## SOLID Architecture & Colocated Unit Tests

Every module under `src/` owns its `CMakeLists.txt`, public `include/`, implementation, and colocated `test/CMakeLists.txt` unit test suite:

```text
.
├── build.py                                        # Unified host TDD + RP2040 firmware orchestrator
├── CMakeLists.txt                                  # Root host build & CTest entry point
├── flake.nix / flake.lock                          # Hermetic Nix environment (zero external installs)
├── docs/
│   ├── WIRING.md                                   # Detailed schematic & pin wiring diagram
│   └── pinout.png                                  # RP2040 board pinout
├── scripts/
│   ├── build                                       # Build host tests + RP2040 UF2 firmware
│   ├── test                                        # Run all colocated unit tests with ASan + UBSan
│   ├── flash                                       # Auto-build, flash UF2, and capture USB-CDC logs
│   └── console                                     # Open interactive picocom console on /dev/ttyACM*
├── src/
│   ├── CMakeLists.txt
│   ├── cmake/p3d.cmake                             # Module & colocated test helpers
│   ├── hal/                                        # Pure abstract HAL interfaces (ISP / DIP)
│   │   ├── CMakeLists.txt
│   │   ├── include/hal/                            # IAdcHardware, IGpioOutput, ISerialIo, ITimeProvider
│   │   ├── test_support/                           # GMock HAL mocks
│   │   └── test/                                   # Colocated HAL contract unit tests
│   ├── platform/rp2040/                            # Concrete RP2040 Pico SDK HAL adapters
│   │   ├── CMakeLists.txt
│   │   ├── include/platform/rp2040/
│   │   └── test/                                   # Colocated off-target unit tests with SDK stubs
│   ├── drivers/                                    # Hardware-independent sensor drivers
│   │   ├── CMakeLists.txt
│   │   ├── include/drivers/                        # IAdcChannelReader, IPotentiometerSensor, AxisId
│   │   ├── test_support/                           # GMock driver mocks
│   │   ├── test/                                   # Colocated driver interface tests
│   │   ├── adc_channel/                            # OversampledAdcChannel + test/CMakeLists.txt
│   │   └── pot_array/                              # PotentiometerArray + test/CMakeLists.txt
│   └── domain/                                     # Pure signal-processing & application logic
│       ├── CMakeLists.txt
│       ├── axis_processor/                         # Deadzone, inversion, clamping, EMA + test/
│       └── app_controller/                         # Control loop, live zeroing, telemetry + test/
└── firmware/
    ├── CMakeLists.txt                              # RP2040 cross-compilation entry point
    └── apps/adc_bringup/                           # RP2040 composition root -> adc_bringup.uf2
```

## Quick Start (Zero External Dependencies)

All compilers (`g++`, `arm-none-eabi-gcc`), `pico-sdk 2.2.0` (with TinyUSB submodules), `gtest`/`gmock`, `cmake`, `ninja`, and `picotool` are pinned in `flake.lock`. All helper scripts automatically invoke `nix develop` if you are not already inside the dev shell:

```bash
# 1. Run all colocated off-target unit tests (with AddressSanitizer + UBSan)
./scripts/test

# 2. Build both host unit tests and RP2040 UF2 firmware
./scripts/build

# 3. Flash the connected Pico over USB-C and capture 5 seconds of live telemetry
./scripts/flash adc_bringup 5

# 4. Open interactive serial console (press 'z' to re-zero center at any time)
./scripts/console
```
