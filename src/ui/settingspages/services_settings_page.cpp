#include "services_settings_page.hpp"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QRegularExpression>
#include <QToolTip>
#include <QVBoxLayout>

#include "core/settings/app_settings.hpp"

using namespace Qt::StringLiterals;
using namespace kisel;

ServicesSettingsPage::ServicesSettingsPage(BaseSettings* settings, QWidget* parent)
    : QWidget(parent)
    , m_settings(settings)
    , m_gamescopeArgsLineEdit(new QLineEdit(this))
{
    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);

    auto* mangohudCheckBox = new QCheckBox("MangoHud"_L1, this);
    mangohudCheckBox->setDisabled(APP_SETTINGS->mangoHudPath().isEmpty());
    mangohudCheckBox->setToolTip(tr("Enable Performance Monitor (requires mangohud to be installed)"));
    mangohudCheckBox->setChecked(m_settings->mangoHudEnabled());
    connect(mangohudCheckBox, &QCheckBox::clicked, this, [this](bool checked) {
        m_settings->setMangoHudEnabled(checked);
    });
    layout->addWidget(mangohudCheckBox);

    auto* obsVkCaptureCheckBox = new QCheckBox("OBS Vulkan Capture"_L1, this);
    obsVkCaptureCheckBox->setDisabled(APP_SETTINGS->obsVkCapturePath().isEmpty());
    obsVkCaptureCheckBox->setToolTip(tr("Enable Vulkan app screen capture for OBS (requires obs-vkcapture to be installed)"));
    obsVkCaptureCheckBox->setChecked(m_settings->obsVkCaptureEnabled());
    connect(obsVkCaptureCheckBox, &QCheckBox::clicked, this, [this](bool checked) {
        m_settings->setObsVkCaptureEnabled(checked);
    });
    layout->addWidget(obsVkCaptureCheckBox);

    auto* xaliaCheckBox = new QCheckBox("Xalia"_L1, this);
    xaliaCheckBox->setToolTip(tr("Enable accessibility controls, such as controlling the application interface with a gamepad"));
    xaliaCheckBox->setChecked(m_settings->xaliaEnabled());
    connect(xaliaCheckBox, &QCheckBox::clicked, this, [this](bool checked) {
        m_settings->setXaliaEnabled(checked);
    });
    layout->addWidget(xaliaCheckBox);

    auto* topGamescopeLine = new QFrame(this);
    topGamescopeLine->setFrameShape(QFrame::HLine);
    layout->addWidget(topGamescopeLine);

    auto* gamescopeCheckBox = new QCheckBox("Gamescope"_L1, this);
    gamescopeCheckBox->setDisabled(APP_SETTINGS->gamescopePath().isEmpty());
    gamescopeCheckBox->setToolTip(tr("Enable Gamescope compositor (requires gamescope to be installed)"));
    gamescopeCheckBox->setChecked(m_settings->gamescopeEnabled());
    layout->addWidget(gamescopeCheckBox);

    m_normalEditPalette = m_gamescopeArgsLineEdit->palette();
    m_errorEditPalette = m_gamescopeArgsLineEdit->palette();
    m_errorEditPalette.setColor(QPalette::Text, QColor(255, 0, 0));

    m_gamescopeArgsLineEdit->setClearButtonEnabled(true);
    m_gamescopeArgsLineEdit->setToolTip(tr("Arguments"));
    m_gamescopeArgsLineEdit->setPlaceholderText(tr("example: -W 1920 -H 1080 -r 60"));
    m_gamescopeArgsLineEdit->setText(m_settings->gamescopeArgs());
    m_gamescopeArgsLineEdit->setEnabled(gamescopeCheckBox->isEnabled() && m_settings->gamescopeEnabled());
    layout->addWidget(m_gamescopeArgsLineEdit);

    connect(gamescopeCheckBox, &QCheckBox::clicked, this, [this](bool checked) {
        m_settings->setGamescopeEnabled(checked);
        m_gamescopeArgsLineEdit->setEnabled(checked);
    });

    connect(m_gamescopeArgsLineEdit, &QLineEdit::textEdited, this, &ServicesSettingsPage::onGamescopeArgsEditFinished);
    connect(m_gamescopeArgsLineEdit, &QLineEdit::editingFinished, this, [this] {
        m_gamescopeArgsLineEdit->setText(m_settings->gamescopeArgs());
        clearGamescopeArgsEditError();
    });
}

void ServicesSettingsPage::onGamescopeArgsEditFinished(QString text)
{
    text = text.trimmed();

    if (text.isEmpty()) {
        clearGamescopeArgsEditError();
        return;
    }

    const QStringList tokens = QProcess::splitCommand(text);

    // 1. gamescope
    if (tokens.contains("gamescope"_L1)) {
        setGamescopeArgsEditError(tr("You don't need to specify \"gamescope\", only its arguments"));
        return;
    }

    // 2. KEY=VALUE
    static const QRegularExpression envPattern(
        R"(^[\p{L}_][\p{L}\p{N}_]*=)"_L1,
        QRegularExpression::UseUnicodePropertiesOption);

    for (const QString& token : tokens) {
        if (!token.startsWith(u'-') && envPattern.match(token).hasMatch()) {
            setGamescopeArgsEditError(tr("Environment variables (KEY=VALUE) are not allowed — use the \"Launch\" section"));
            return;
        }

        // 3. -arg
        if (!token.startsWith(u'-') && token.contains(u'=')) {
            setGamescopeArgsEditError(tr("Arguments must start with '-'"));
            return;
        }
    }

    clearGamescopeArgsEditError();
}

void ServicesSettingsPage::setGamescopeArgsEditError(const QString& message)
{
    m_gamescopeArgsLineEdit->setPalette(m_errorEditPalette);
    m_gamescopeArgsLineEdit->setToolTip(message);

    if (m_gamescopeArgsLineEdit->hasFocus()) {
        QToolTip::showText(mapToGlobal(m_gamescopeArgsLineEdit->pos()), message, m_gamescopeArgsLineEdit);
    }
}

void ServicesSettingsPage::clearGamescopeArgsEditError()
{
    m_gamescopeArgsLineEdit->setPalette(m_normalEditPalette);
    setToolTip(QString());
    QToolTip::hideText();
    m_settings->setGamescopeArgs(m_gamescopeArgsLineEdit->text().trimmed());
}
