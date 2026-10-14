#pragma once

#include <QWidget>

#include "core/settings/base_settings.hpp"

namespace kisel {
class PlatformSettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit PlatformSettingsPage(BaseSettings* baseSettings, QWidget* parent = nullptr);

private:
    static const QStringList& storeList();

    BaseSettings* m_settings;
};
}