#include "ui/widgets/FirmwareUpdaterWidget.hpp"

#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace p3d::ui
{
    FirmwareUpdaterWidget::FirmwareUpdaterWidget(QWidget* parent)
        : QWidget(parent)
    {
        auto* rootLayout = new QVBoxLayout(this);
        rootLayout->setContentsMargins(0, 0, 0, 0);

        auto* group = new QGroupBox(
            QStringLiteral("Buttonless USB-C Firmware Updater (Enclosed Housing — No BOOTSEL Press)"),
            this);
        auto* vbox = new QVBoxLayout(group);

        auto* pathRow = new QHBoxLayout();
        pathRow->addWidget(new QLabel(QStringLiteral("UF2 Image:"), group));
        m_pathEdit = new QLineEdit(group);
        m_pathEdit->setPlaceholderText(
            QStringLiteral("firmware/build/apps/adc_bringup/adc_bringup.uf2"));
        m_browseBtn = new QPushButton(QStringLiteral("Browse..."), group);
        pathRow->addWidget(m_pathEdit, 1);
        pathRow->addWidget(m_browseBtn);
        vbox->addLayout(pathRow);

        auto* actionRow = new QHBoxLayout();
        m_flashBtn      = new QPushButton(
            QStringLiteral("Flash Firmware Over USB (Buttonless)"), group);
        m_statusBadge = new QLabel(QStringLiteral("IDLE"), group);
        m_statusBadge->setAlignment(Qt::AlignCenter);
        m_statusBadge->setMinimumWidth(140);
        actionRow->addWidget(m_flashBtn, 1);
        actionRow->addWidget(m_statusBadge);
        vbox->addLayout(actionRow);

        m_logView = new QPlainTextEdit(group);
        m_logView->setReadOnly(true);
        m_logView->setMaximumHeight(95);
        m_logView->setPlaceholderText(
            QStringLiteral("Buttonless firmware update log (sends software BOOTSEL + picotool load -f -v -x)..."));
        vbox->addWidget(m_logView);

        rootLayout->addWidget(group);

        connect(m_browseBtn,
                &QPushButton::clicked,
                this,
                &FirmwareUpdaterWidget::onBrowseClicked);
        connect(m_flashBtn,
                &QPushButton::clicked,
                this,
                &FirmwareUpdaterWidget::onFlashClicked);
    }

    QString FirmwareUpdaterWidget::uf2FilePath() const
    {
        return m_pathEdit->text().trimmed();
    }

    QString FirmwareUpdaterWidget::logText() const
    {
        return m_logView->toPlainText();
    }

    void FirmwareUpdaterWidget::setUf2FilePath(const QString& path)
    {
        m_pathEdit->setText(path);
    }

    void FirmwareUpdaterWidget::setUpdatingState(bool updating)
    {
        m_flashBtn->setEnabled(!updating);
        m_browseBtn->setEnabled(!updating);
        m_pathEdit->setEnabled(!updating);
        if (updating)
        {
            m_statusBadge->setText(QStringLiteral("FLASHING..."));
        }
    }

    void FirmwareUpdaterWidget::appendLogLine(const QString& line)
    {
        m_logView->appendPlainText(line);
    }

    void FirmwareUpdaterWidget::setUpdateResult(bool success, const QString& summary)
    {
        setUpdatingState(false);
        m_statusBadge->setText(success ? QStringLiteral("UPDATE OK")
                                       : QStringLiteral("UPDATE FAILED"));
        if (!summary.isEmpty())
        {
            m_logView->appendPlainText(summary);
        }
    }

    void FirmwareUpdaterWidget::onBrowseClicked()
    {
        const QString selected = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("Select RP2040 UF2 Firmware Image"),
            m_pathEdit->text(),
            QStringLiteral("UF2 Firmware Images (*.uf2);;All Files (*)"));
        if (!selected.isEmpty())
        {
            m_pathEdit->setText(selected);
        }
    }

    void FirmwareUpdaterWidget::onFlashClicked()
    {
        emit firmwareUpdateRequested(m_pathEdit->text().trimmed());
    }
} // namespace p3d::ui
