#pragma once

#include <cstdint>
#include "PicoSdkStubState.hpp"

inline bool tud_hid_ready()
{
    return p3d::test::sdk_stub::state().hidReady;
}

inline bool tud_hid_report(std::uint8_t report_id, const void* /*report*/, std::uint16_t /*len*/)
{
    ++p3d::test::sdk_stub::state().hidReportCount;
    p3d::test::sdk_stub::state().lastHidReportId = report_id;
    return true;
}

inline void tud_task()
{
}
