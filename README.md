# pico-3d-mouse

Self-contained RP2040 3D mouse firmware and cross-platform Qt6 MVC Manufacturing Tester, Calibration & Buttonless Firmware Updater built in **100% C++17**, following **SOLID** design principles and full off-target **TDD** (GoogleTest + GMock).

## Key Features

- **Composite USB-C Device (`0x2E8A:0x000A`)**:
  - **6-DOF Multi-Axis Controller HID** (Interface 3): Standard USB HID Usage Page `0x01`, Usage `0x08` reporting 16-bit signed translation (`X, Y, Z`) and rotation (`Rx, Ry, Rz`) vectors in `[-500, +500]`.
  - **USB-CDC Telemetry & Calibration Serial** (Interfaces 0 & 1): Streams live oversampled 12-bit ADC readings, millivolts, conditioned deltas, and scaled outputs, and accepts line-based calibration commands (`ZERO`, `CAL_MINMAX_START`, `CAL_MINMAX_END`, `SET_DZ`, `SET_INV`, `SAVE_CAL`, `LOAD_CAL`, `RESET_CAL`, `GET_CAL`, `BOOTSEL`).
  - **RP2040 USB Reset Interface** (Interface 2): Supports 100% buttonless software warm-reset into USB-BOOT (`BOOTSEL`) mode.
- **Non-Volatile QSPI Flash Calibration Store**:
  - Persists per-axis center bias, min/max mechanical travel limits, deadzone counts, and polarity inversion to the final 4 KB sector of RP2040 internal QSPI Flash, protected by a magic header, schema version, and **IEEE 802.3 CRC-32** checksum.
- **Qt6 MVC Manufacturing Tester, Calibration & Buttonless Firmware Updater (`./scripts/ui`)**:
  - **100% Event-Driven (Zero Polling)**: Powered exclusively by Qt Signals & Slots (`QSerialPort::readyRead`, `QProcess` signals, and MVC model change notifications).
  - **Custom Decoupled SOLID Widgets**:
    - `GimbalRadarWidget`: Custom `QPainter` 2D X/Y deflection radar + concentric Twist rotation arc.
    - `AxisGaugeWidget`: Custom `QPainter` 12-bit ADC channel bar (`0..4095`), millivolts, center marker, deadzone band, min/max sweep watermarks, and per-axis Manufacturing Pass/Pending badge.
    - `CalibrationPanelWidget`: Interactive center zeroing, min/max sweep capture, per-axis deadzone spinboxes, polarity inversion checkboxes, and QSPI Flash save/load/reset controls.
    - `FirmwareUpdaterWidget`: **Buttonless USB-C Firmware Updater** designed for enclosed housings where the physical `BOOTSEL` button is inaccessible.

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
    Host["Host Computer"] <==>|"USB-C (6-DOF HID + USB-CDC + Reset)"| USB["USB-C Port"]

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

Every module directory under `src/` owns its `CMakeLists.txt`, public `include/`, implementation, and colocated `test/CMakeLists.txt` unit test suite (58 unit tests across 15 suites):

```text
.
├── CMakeLists.txt                                  # Root host build & CTest entry point
├── flake.nix / flake.lock                          # Hermetic Nix environment (zero external installs)
├── docs/
│   ├── WIRING.md                                   # Detailed schematic & pin wiring diagram
│   └── pinout.png                                  # RP2040 board pinout
├── scripts/
│   ├── build                                       # Build host tests, Qt6 UI, and RP2040 UF2 firmware
│   ├── test                                        # Run all 58 colocated unit tests with ASan + UBSan
│   ├── flash                                       # Buttonless auto-build, flash UF2, and capture logs
│   ├── ui                                          # Launch Qt6 MVC Manufacturing Tester & Updater UI
│   └── console                                     # Open interactive picocom console on /dev/ttyACM*
├── src/
│   ├── CMakeLists.txt
│   ├── cmake/p3d.cmake                             # Module & colocated test helpers
│   ├── hal/                                        # Pure abstract HAL interfaces + test/
│   │   ├── include/hal/                            # IAdcHardware, IGpioOutput, ISerialIo, ITimeProvider,
│   │   │                                           # INvmStorage, IHidTransport, IBootControl
│   │   ├── test_support/                           # GMock HAL mocks
│   │   └── test/                                   # Colocated HAL contract unit tests
│   ├── platform/rp2040/                            # Concrete RP2040 Pico SDK HAL adapters + test/
│   ├── drivers/                                    # Hardware-independent sensor drivers + test/
│   │   ├── adc_channel/                            # OversampledAdcChannel + test/
│   │   └── pot_array/                              # PotentiometerArray + test/
│   ├── domain/                                     # Pure signal-processing & application logic
│   │   ├── axis_processor/                         # Deadzone, inversion, clamping, EMA + test/
│   │   ├── calibration_store/                      # CRC-32 QSPI Flash calibration persistence + test/
│   │   ├── command_parser/                         # Zero-allocation serial command parser + test/
│   │   └── app_controller/                         # Control loop, 6-DOF HID, calibration, BOOTSEL + test/
│   └── ui/                                         # Cross-platform Qt6 MVC Desktop Application
│       ├── interfaces/                             # ISerialTransport, IFirmwareUpdater, IProcessRunner + test/
│       ├── services/                               # QtSerialTransport, PicotoolFirmwareUpdater + test/
│       ├── models/                                 # DeviceTelemetryModel (telemetry, cal, mfg checks) + test/
│       ├── widgets/                                # Decoupled SOLID Custom Widgets (each with test/)
│       │   ├── gimbal_radar/                       # GimbalRadarWidget + test/
│       │   ├── axis_gauge/                         # AxisGaugeWidget + test/
│       │   ├── calibration_panel/                  # CalibrationPanelWidget + test/
│       │   └── firmware_updater/                   # FirmwareUpdaterWidget + test/
│       ├── controllers/                            # ManufacturingController + test/
│       └── app/                                    # MainWindow + pico_3d_mouse_ui executable + test/
└── firmware/
    ├── CMakeLists.txt                              # RP2040 cross-compilation entry point
    └── apps/adc_bringup/                           # RP2040 composition root -> adc_bringup.uf2
```

## Quick Start (Zero External Dependencies)

All compilers (`g++`, `arm-none-eabi-gcc`), `pico-sdk 2.2.0` (with TinyUSB submodules), `qt6` (`qtbase`, `qtserialport`), `gtest`/`gmock`, `cmake`, `ninja`, and `picotool` are pinned in `flake.lock`. All helper scripts automatically invoke `nix develop` if you are not already inside the dev shell:

```bash
# 1. Run all 58 colocated off-target unit tests (with AddressSanitizer + UBSan)
./scripts/test

# 2. Build host unit tests, Qt6 MVC desktop app, and RP2040 UF2 firmware
./scripts/build

# 3. Flash the connected Pico over USB-C (100% buttonless) and capture 5s of telemetry
./scripts/flash adc_bringup 5

# 4. Launch the Qt6 MVC Manufacturing Tester, Calibration & Buttonless Firmware Updater UI
./scripts/ui /dev/ttyACM0
```
