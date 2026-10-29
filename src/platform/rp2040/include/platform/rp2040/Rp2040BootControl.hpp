#pragma once

#include "hal/IBootControl.hpp"

namespace p3d::platform::rp2040
{
    class Rp2040BootControl final : public hal::IBootControl
    {
    public:
        void enterBootloader() override;
    };
}
