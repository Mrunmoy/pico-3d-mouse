#include "ui/models/DeviceTelemetryModel.hpp"

#include <gtest/gtest.h>

namespace p3d::ui
{
    namespace
    {
        TEST(DeviceTelemetryModelTest, ParsesLiveTelemetryAndUpdatesWatermarks)
        {
            DeviceTelemetryModel model;
            int                  telemetryCount = 0;

            QObject::connect(&model,
                             &DeviceTelemetryModel::telemetryUpdated,
                             [&]() { ++telemetryCount; });

            model.onSerialLineReceived(
                "FWD_BCK(ADC0)[raw=2117 1706mV d=   +5 out=  +1]  "
                "LFT_RGT(ADC1)[raw=2111 1701mV d=  -12 out=  +0]  "
                "ROT(ADC2)[raw=2109 1699mV d= +100 out= +24]");

            EXPECT_EQ(telemetryCount, 1);
            EXPECT_EQ(model.axisLive(0).rawCounts, 2117U);
            EXPECT_EQ(model.axisLive(0).millivolts, 1706U);
            EXPECT_EQ(model.axisLive(0).deltaCounts, 5);
            EXPECT_EQ(model.axisLive(0).scaledOutput, 1);

            EXPECT_EQ(model.axisLive(1).rawCounts, 2111U);
            EXPECT_EQ(model.axisLive(1).deltaCounts, -12);
            EXPECT_EQ(model.axisLive(1).scaledOutput, 0);

            EXPECT_EQ(model.axisLive(2).rawCounts, 2109U);
            EXPECT_EQ(model.axisLive(2).deltaCounts, 100);
            EXPECT_EQ(model.axisLive(2).scaledOutput, 24);
        }

        TEST(DeviceTelemetryModelTest, ParsesCalibrationStatusLineAndEmitsEvent)
        {
            DeviceTelemetryModel model;
            QString              calEvent;

            QObject::connect(&model,
                             &DeviceTelemetryModel::calibrationUpdated,
                             [&](const QString& ev) { calEvent = ev; });

            model.onSerialLineReceived(
                "@CAL SAVED_OK CH0[c=2117 min=120 max=3980 dz=15 inv=1] "
                "CH1[c=2111 min=90 max=4010 dz=12 inv=0] "
                "CH2[c=2109 min=200 max=3900 dz=20 inv=1]");

            EXPECT_EQ(calEvent, "SAVED_OK");
            EXPECT_EQ(model.axisCalibration(0).centerCounts, 2117U);
            EXPECT_EQ(model.axisCalibration(0).minCounts, 120U);
            EXPECT_EQ(model.axisCalibration(0).maxCounts, 3980U);
            EXPECT_EQ(model.axisCalibration(0).deadzoneCounts, 15U);
            EXPECT_TRUE(model.axisCalibration(0).invert);

            EXPECT_EQ(model.axisCalibration(1).centerCounts, 2111U);
            EXPECT_FALSE(model.axisCalibration(1).invert);

            EXPECT_EQ(model.axisCalibration(2).deadzoneCounts, 20U);
            EXPECT_TRUE(model.axisCalibration(2).invert);
        }

        TEST(DeviceTelemetryModelTest, ComputesManufacturingPassVerdictOnlyAfterFullSweepOnAllAxes)
        {
            DeviceTelemetryModel model;
            bool                 verdictSignal = false;

            QObject::connect(&model,
                             &DeviceTelemetryModel::manufacturingVerdictChanged,
                             [&](bool passed) { verdictSignal = passed; });

            // Resting sample: center is in spec, but min/max travel not yet swept
            model.onSerialLineReceived(
                "FWD_BCK(ADC0)[raw=2048 1650mV d= +0 out= +0]  "
                "LFT_RGT(ADC1)[raw=2050 1651mV d= +0 out= +0]  "
                "ROT(ADC2)[raw=2045 1647mV d= +0 out= +0]");
            EXPECT_TRUE(model.axisVerdict(0).centerInSpec);
            EXPECT_FALSE(model.axisVerdict(0).passed());
            EXPECT_FALSE(model.allAxesPassed());

            // Sweep negative end on all 3 axes
            model.onSerialLineReceived(
                "FWD_BCK(ADC0)[raw=250 201mV d=-1798 out=-500]  "
                "LFT_RGT(ADC1)[raw=300 241mV d=-1750 out=-500]  "
                "ROT(ADC2)[raw=400 322mV d=-1645 out=-500]");
            EXPECT_TRUE(model.axisVerdict(0).negativeTravelReached);
            EXPECT_FALSE(model.allAxesPassed());

            // Sweep positive end on all 3 axes
            model.onSerialLineReceived(
                "FWD_BCK(ADC0)[raw=3850 3102mV d=+1802 out=+500]  "
                "LFT_RGT(ADC1)[raw=3900 3142mV d=+1850 out=+500]  "
                "ROT(ADC2)[raw=3700 2981mV d=+1655 out=+500]");
            EXPECT_TRUE(model.axisVerdict(0).positiveTravelReached);
            EXPECT_TRUE(model.allAxesPassed());
            EXPECT_TRUE(verdictSignal);

            // Resetting watermarks clears the pass verdict for the next device under test
            model.resetWatermarks();
            EXPECT_FALSE(model.allAxesPassed());
            EXPECT_FALSE(verdictSignal);
        }
    } // namespace
} // namespace p3d::ui
