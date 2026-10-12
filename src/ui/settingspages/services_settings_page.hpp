#pragma once

#include <QLineEdit>

#include "core/settings/base_settings.hpp"

namespace kisel {
class ServicesSettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit ServicesSettingsPage(BaseSettings* settings, QWidget* parent = nullptr);

private slots:
    void onGamescopeArgsEditFinished(QString text);

private:
    void setGamescopeArgsEditError(const QString& message);
    void clearGamescopeArgsEditError();

    QPalette m_normalEditPalette;
    QPalette m_errorEditPalette;
    BaseSettings* m_settings;
    QLineEdit* m_gamescopeArgsLineEdit;
};
}