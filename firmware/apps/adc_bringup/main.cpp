// First hardware bringup target for pico-3d-mouse (RP2040).
//
// Responsibilities:
//   1. Initialize ADC0 (GP26), ADC1 (GP27), and ADC2 (GP28) connected to the
//      three potentiometer wipers between 3V3(OUT) and AGND.
//   2. Auto-zero the resting center position on startup (and on 'z' over USB serial).
//   3. Stream raw 12-bit counts, millivolts, and center-relative deltas over USB-CDC
//      at 20 Hz while toggling the onboard status LED (GP25).

#include <cstdint>
#include <cstdio>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "board/pins.hpp"
#include "drivers/pots.hpp"

namespace
{
    constexpr std::uint32_t kSamplePeriodMs = 50;  // 20 Hz telemetry rate

    void initStatusLed()
    {
        gpio_init(board::pins::kStatusLedGpio);
        gpio_set_dir(board::pins::kStatusLedGpio, GPIO_OUT);
        gpio_put(board::pins::kStatusLedGpio, 1);
    }

    void printCalibrationBanner(const drivers::pots::Reader& pots)
    {
        const auto& c = pots.centerCounts();
        std::printf(
            "\r\n[pico-3d-mouse] Center calibrated: "
            "CH0(GP26)=%4u  CH1(GP27)=%4u  CH2(GP28)=%4u  (press 'z' to re-zero)\r\n",
            static_cast<unsigned>(c[0]),
            static_cast<unsigned>(c[1]),
            static_cast<unsigned>(c[2]));
    }
}

int main()
{
    stdio_init_all();
    initStatusLed();

    drivers::pots::Reader pots{};
    pots.init();

    // Give the analog rails a brief moment to settle after power-on before zeroing.
    sleep_ms(100);
    pots.calibrateCenter();

    std::uint32_t tick = 0;
    bool ledState = true;

    while (true)
    {
        const int ch = getchar_timeout_us(0);
        if (ch == 'z' || ch == 'Z')
        {
            pots.calibrateCenter();
            printCalibrationBanner(pots);
        }

        const drivers::pots::Sample s = pots.read();

        std::printf(
            "CH0[raw=%4u %4umV d=%+5d]  "
            "CH1[raw=%4u %4umV d=%+5d]  "
            "CH2[raw=%4u %4umV d=%+5d]\r\n",
            static_cast<unsigned>(s.raw[0]),
            static_cast<unsigned>(s.millivolts[0]),
            static_cast<int>(s.delta[0]),
            static_cast<unsigned>(s.raw[1]),
            static_cast<unsigned>(s.millivolts[1]),
            static_cast<int>(s.delta[1]),
            static_cast<unsigned>(s.raw[2]),
            static_cast<unsigned>(s.millivolts[2]),
            static_cast<int>(s.delta[2]));

        if ((++tick % 10U) == 0U)
        {
            ledState = !ledState;
            gpio_put(board::pins::kStatusLedGpio, ledState ? 1 : 0);
        }

        sleep_ms(kSamplePeriodMs);
    }
}
