#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QWidget>

namespace p3d::ui
{
    // Standalone decoupled widget for buttonless over-USB firmware updates.
    // Zero coupling to IFirmwareUpdater, ISerialTransport, or controllers.
    class FirmwareUpdaterWidget final : public QWidget
    {
        Q_OBJECT

    public:
        explicit FirmwareUpdaterWidget(QWidget* parent = nullptr);
        ~FirmwareUpdaterWidget() override = default;

        QString      uf2FilePath() const;
        QPushButton* flashButton() const { return m_flashBtn; }
        QString      logText() const;

    public slots:
        void setUf2FilePath(const QString& path);
        void setUpdatingState(bool updating);
        void appendLogLine(const QString& line);
        void setUpdateResult(bool success, const QString& summary);

    signals:
        void firmwareUpdateRequested(const QString& uf2FilePath);

    private slots:
        void onBrowseClicked();
        void onFlashClicked();

    private:
        QLineEdit*      m_pathEdit{ nullptr };
        QPushButton*    m_browseBtn{ nullptr };
        QPushButton*    m_flashBtn{ nullptr };
        QLabel*         m_statusBadge{ nullptr };
        QPlainTextEdit* m_logView{ nullptr };
    };
} // namespace p3d::ui
