/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "waylandbackend.h"

#include "waylandconfig.h"
#include "waylandoutput.h"

#include <configmonitor.h>
#include <mode.h>

#include <QSettings>
#include <QStandardPaths>

using namespace KScreen;

Q_LOGGING_CATEGORY(KSCREEN_WAYLAND, "kscreen.kwayland")

WaylandBackend::WaylandBackend()
    : KScreen::AbstractBackend()
    , m_internalConfig(new WaylandConfig(this))
{
    qCDebug(KSCREEN_WAYLAND) << "Loading Wayland backend.";

    connect(m_internalConfig, &WaylandConfig::configChanged, this, [this]() {
        Q_EMIT configChanged(m_internalConfig->currentConfig());
    });
}

QString WaylandBackend::name() const
{
    return QStringLiteral("kwayland");
}

QString WaylandBackend::serviceName() const
{
    return QStringLiteral("org.kde.KScreen.Backend.KWayland");
}

ConfigPtr WaylandBackend::config() const
{
    // Note: This should ONLY be called from GetConfigOperation!
    return m_internalConfig->currentConfig();
}

void WaylandBackend::setConfig(const KScreen::ConfigPtr &newconfig)
{
    if (!newconfig) {
        return;
    }
    m_internalConfig->applyConfig(newconfig);
}

QByteArray WaylandBackend::edid(int outputId) const
{
    WaylandOutput *output = m_internalConfig->outputMap().value(outputId);
    if (!output) {
        return QByteArray();
    }
    return output->outputDevice()->edid();
}

bool WaylandBackend::isValid() const
{
    return m_internalConfig->isInitialized();
}
