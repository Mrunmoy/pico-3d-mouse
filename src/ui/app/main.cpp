#include "ui/app/MainWindow.hpp"
#include "ui/controllers/ManufacturingController.hpp"
#include "ui/models/DeviceTelemetryModel.hpp"
#include "ui/services/PicotoolFirmwareUpdater.hpp"
#include "ui/services/QtProcessRunner.hpp"
#include "ui/services/QtSerialTransport.hpp"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("pico-3d-mouse-tester"));
    app.setOrganizationName(QStringLiteral("Mrunmoy"));

    p3d::ui::QtSerialTransport       serialTransport{};
    p3d::ui::QtProcessRunner         processRunner{};
    p3d::ui::PicotoolFirmwareUpdater firmwareUpdater{ processRunner, /*verifyFileExists=*/true };
    p3d::ui::DeviceTelemetryModel    telemetryModel{};
    p3d::ui::ManufacturingController controller{
        serialTransport, firmwareUpdater, telemetryModel
    };

    p3d::ui::MainWindow window{
        serialTransport, firmwareUpdater, telemetryModel, controller
    };

    // Auto-connect if a command-line serial port argument was passed (e.g. ./scripts/ui /dev/ttyACM0)
    const QStringList args = app.arguments();
    if (args.size() > 1 && !args[1].startsWith('-'))
    {
        controller.requestConnect(args[1]);
    }

    window.show();
    return app.exec();
}
