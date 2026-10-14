#pragma once

#include <QSettings>

#include "base_settings.hpp"
#include "core/settings/app_settings.hpp"

namespace kisel {
class PrefixSettings : public BaseSettings {
public:
    explicit PrefixSettings(const QString& path, QObject* parent = nullptr);

    // Launch
    [[nodiscard]] QString customRunCommand() const override;
    [[nodiscard]] QMap<QString, QVariant> envMap() override;

    // Services
    [[nodiscard]] bool mangoHudEnabled() const override;
    [[nodiscard]] bool gamescopeEnabled() const override;
    [[nodiscard]] QString gamescopeArgs() const override;
    [[nodiscard]] bool obsVkCaptureEnabled() const override;
    [[nodiscard]] bool xaliaEnabled() const override;

    // Compatibility
    [[nodiscard]] QString ctPath() const override;
    [[nodiscard]] bool nvapiEnabled() const override;
    [[nodiscard]] bool waylandEnabled() const override;
    [[nodiscard]] bool hdrEnabled() const override;
    [[nodiscard]] bool wow64Enabled() const override;
    [[nodiscard]] bool sdlInputEnabled() const override;
    [[nodiscard]] bool openglEnabled() const override;

    // Platform
    [[nodiscard]] bool steamEnabled() const override;
    [[nodiscard]] bool steamEnvEnabled() const override;
    [[nodiscard]] bool steamOverlayEnabled() const override;
    [[nodiscard]] bool onlineFixEnabled() const override;
    // Platform/Flatpak
    [[nodiscard]] bool protonSandboxDisabled() const override;

private:
    AppSettings* m_appSettings;
};
}