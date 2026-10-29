#pragma once

namespace p3d::hal
{
    // Pure hardware abstraction for a single digital output pin (e.g. status LED).
    class IGpioOutput
    {
    public:
        virtual ~IGpioOutput() = default;

        virtual bool init() = 0;
        virtual bool write(bool level) = 0;
    };
}
