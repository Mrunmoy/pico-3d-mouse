#include "ui/widgets/FirmwareUpdaterWidget.hpp"

#include <QApplication>
#include <gtest/gtest.h>

namespace p3d::ui
{
    namespace
    {
        QApplication* ensureApp()
        {
            static int   argc = 1;
            static char  arg0[] = "test_firmware_updater_widget";
            static char* argv[] = { arg0, nullptr };
            static QApplication app(argc, argv);
            return &app;
        }

        TEST(FirmwareUpdaterWidgetTest, EmitsFirmwareUpdateRequestedAndUpdatesLogAndState)
        {
            ensureApp();
            FirmwareUpdaterWidget widget;

            QString requestedUf2;
            QObject::connect(&widget,
                             &FirmwareUpdaterWidget::firmwareUpdateRequested,
                             [&](const QString& path) { requestedUf2 = path; });

            widget.setUf2FilePath("firmware/build/apps/adc_bringup/adc_bringup.uf2");
            widget.flashButton()->click();

            EXPECT_EQ(requestedUf2, "firmware/build/apps/adc_bringup/adc_bringup.uf2");

            widget.setUpdatingState(true);
            EXPECT_FALSE(widget.flashButton()->isEnabled());

            widget.appendLogLine("Sending software BOOTSEL...");
            widget.setUpdateResult(true, "Done");

            EXPECT_TRUE(widget.flashButton()->isEnabled());
            EXPECT_TRUE(widget.logText().contains("Sending software BOOTSEL..."));
        }
    } // namespace
} // namespace p3d::ui
