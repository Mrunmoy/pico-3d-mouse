// Composition root for the RP2040 3D Mouse ADC bringup firmware.
//
// Axis wiring:
//   - Forward / Backward : ADC0 (GP26, Pin 31)
//   - Left / Right       : ADC1 (GP27, Pin 32)
//   - Rotation (Twist)   : ADC2 (GP28, Pin 34)
//   - Potentiometer +V   : 3V3(OUT) (Pin 36)
//   - Potentiometer GND  : AGND (Pin 33)

#include "domain/AppController.hpp"
#include "drivers/OversampledAdcChannel.hpp"
#include "drivers/PotentiometerArray.hpp"
#include "platform/rp2040/BoardPins.hpp"
#include "platform/rp2040/Rp2040AdcHardware.hpp"
#include "platform/rp2040/Rp2040GpioOutput.hpp"
#include "platform/rp2040/Rp2040TimeProvider.hpp"
#include "platform/rp2040/Rp2040UsbSerialIo.hpp"

int main()
{
    namespace rp = p3d::platform::rp2040;

    // ── Platform HAL adapters ────────────────────────────────────
    rp::Rp2040AdcHardware  adcHw{};
    rp::Rp2040GpioOutput   statusLed{ rp::pins::kStatusLedGpio };
    rp::Rp2040UsbSerialIo  serial{};
    rp::Rp2040TimeProvider time{};

    // ── Oversampled ADC channel readers ──────────────────────────
    p3d::drivers::OversampledAdcChannel fwdBackAdc{
        adcHw, p3d::drivers::AdcChannelConfig{ 0, 16, true, 3300, 4095 }
    };
    p3d::drivers::OversampledAdcChannel leftRightAdc{
        adcHw, p3d::drivers::AdcChannelConfig{ 1, 16, true, 3300, 4095 }
    };
    p3d::drivers::OversampledAdcChannel rotationAdc{
        adcHw, p3d::drivers::AdcChannelConfig{ 2, 16, true, 3300, 4095 }
    };

    p3d::drivers::PotentiometerArray pots{ fwdBackAdc, leftRightAdc, rotationAdc };

    // ── Application controller ───────────────────────────────────
    p3d::domain::AppConfig cfg{};
    cfg.startupSettleMs        = 100;
    cfg.calibrationSweeps      = 64;
    cfg.samplePeriodMs         = 50;
    cfg.ledToggleIntervalTicks = 10;
    for (auto& ax : cfg.axisConfigs)
    {
        ax.deadzoneCounts      = 12;
        ax.maxDeflectionCounts = 2035;
        ax.outputScaleMax      = 500;
        ax.emaAlpha            = 0.35f;
        ax.invert              = false;
    }

    p3d::domain::AppController app{ pots, statusLed, serial, time, cfg };

    (void)app.start();

    while (true)
    {
        (void)app.tick();
    }
}
