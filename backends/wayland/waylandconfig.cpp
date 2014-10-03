/*************************************************************************************
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>                                  *
 *  Copyright 2013 Martin Gräßlin <mgraesslin@kde.org>                               *
 *                                                                                   *
 *  This library is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU Lesser General Public                       *
 *  License as published by the Free Software Foundation; either                     *
 *  version 2.1 of the License, or (at your option) any later version.               *
 *                                                                                   *
 *  This library is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                *
 *  Lesser General Public License for more details.                                  *
 *                                                                                   *
 *  You should have received a copy of the GNU Lesser General Public                 *
 *  License along with this library; if not, write to the Free Software              *
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA       *
 *************************************************************************************/

#include "waylandconfig.h"
#include "waylandoutput.h"
#include "waylandscreen.h"
#include "waylandbackend.h"

#include <QThread>

// Wayland
#include <wayland-client-protocol.h>

// KWayland
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>

#include <configmonitor.h>
#include <mode.h>


using namespace KScreen;


WaylandConfig::WaylandConfig(QObject *parent)
    : QObject(parent)
    , m_screen(new WaylandScreen(this))
    , m_blockSignals(true)
    , m_registryInitialized(false)

{
    qCDebug(KSCREEN_WAYLAND) << " Config creating.";
    connect(this, &WaylandConfig::initialized, &m_syncLoop, &QEventLoop::quit);
    initConnection();
    m_syncLoop.exec();
    qDebug() << "CTOR Done. Outputs:" << m_outputMap;
//     m_blockSignals = false;
}

WaylandConfig::~WaylandConfig()
{
    qDebug() << "Byebye";
    Q_FOREACH (auto output, m_outputMap.values()) {
        delete output;
    }
}

void WaylandConfig::initConnection()
{
    m_connection = new KWayland::Client::ConnectionThread;
    QThread *thread = new QThread;
    m_connection->moveToThread(thread);
    thread->start();

    connect(m_connection, &KWayland::Client::ConnectionThread::connected, this, &WaylandConfig::setupRegistry, Qt::QueuedConnection);

    connect(m_connection, &KWayland::Client::ConnectionThread::failed, [=] {
        qDebug() << "Failed to connect to Wayland server at socket:" << m_connection->socketName();
    });
    m_connection->initConnection();
}

void WaylandConfig::setupRegistry()
{
    qDebug() << "Connected to Wayland server at socket:" << m_connection->socketName();

    m_queue = new KWayland::Client::EventQueue(this);
    m_queue->setup(m_connection);

    m_registry = new KWayland::Client::Registry(this);
    m_registry->setEventQueue(m_queue);

    connect(m_registry, &KWayland::Client::Registry::outputAnnounced, this, &WaylandConfig::addOutput, Qt::DirectConnection);

    connect(m_registry, &KWayland::Client::Registry::sync, [=] {
        qDebug() << "Registry::Sync arrived in Backend!:";
        m_registryInitialized = true;
        checkInitialized();
    });

    m_registry->create(m_connection);
    m_registry->setup();
}


void WaylandConfig::addOutput(quint32 name, quint32 version)
{
    qDebug() << "Adding output" << name;
    if (!m_blockSignals) {
        m_initializingOutputs << name;
    }
    if (m_outputMap.keys().contains(name)) {
        qDebug() << "Output already known";
        return;
    }

    WaylandOutput *waylandoutput = new WaylandOutput(this);
    waylandoutput->setId(name);
    waylandoutput->setup(m_registry->bindOutput(name, version));

    connect(waylandoutput, &WaylandOutput::complete, [=]{
        m_outputMap[waylandoutput->id()] = waylandoutput;
        m_initializingOutputs.removeAll(name);
        checkInitialized();
        if (!m_blockSignals) {
        } else {
            qCDebug(KSCREEN_WAYLAND) << "added output complete .. notifyUpdate()" << name;
            KScreen::ConfigMonitor::instance()->notifyUpdate();
        }
    });
}

void WaylandConfig::checkInitialized()
{
    qDebug() << "CHECK: " << m_registryInitialized << m_initializingOutputs.isEmpty() << m_outputMap;
    if (m_registryInitialized && m_initializingOutputs.isEmpty() && m_outputMap.count()) {
        qDebug() << "We're done!";
        emit initialized();
        m_blockSignals = false;
        //m_syncLoop.quit();
    };
}


Config* WaylandConfig::toKScreenConfig() const
{
    Config *config = new Config();
    config->setScreen(m_screen->toKScreenScreen(config));
    updateKScreenConfig(config);
    return config;
}

int WaylandConfig::outputId(KWayland::Client::Output *wlo)
{
    QList<int> ids;
    Q_FOREACH (auto output, m_outputMap.values()) {
        if (wlo == output) {
            return output->id();
        }
    }
    m_lastOutputId++;
    return m_lastOutputId;
}

void WaylandConfig::removeOutput(quint32 id)
{
    qCDebug(KSCREEN_WAYLAND) << "output screen Removed!!! .." << id << m_outputMap[id];
    // Find output matching the QScreen object and remove it
    int removedOutputId = -1;
    Q_FOREACH (auto output, m_outputMap.values()) {
//         if (output->qscreen() == qscreen) {
//             qDebug() << "Found output matching the qscreen " << output;
//             removedOutputId = output->id();
//             m_outputMap.remove(removedOutputId);
//             delete output;
//         }
    }
    if (!m_blockSignals) {
        KScreen::ConfigMonitor::instance()->notifyUpdate();
    }
}

void WaylandConfig::updateKScreenConfig(Config* config) const
{
    qCDebug(KSCREEN_WAYLAND) << "===>> updateKScreenConfig";
    m_screen->updateKScreenScreen(config->screen());

    //Removing removed outputs
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH (KScreen::Output *output, outputs) {
        if (!m_outputMap.keys().contains(output->id())) {
            config->removeOutput(output->id());
        }
    }
    qCDebug(KSCREEN_WAYLAND) << "updateKScreenConfig" << m_outputMap.keys();

    // Add KScreen::Outputs that aren't in the list yet, handle primaryOutput
    Q_FOREACH (auto output, m_outputMap.values()) {

        KScreen::Output *kscreenOutput = config->output(output->id());

        if (!kscreenOutput) {
            kscreenOutput = output->toKScreenOutput(config);
            config->addOutput(kscreenOutput);
            qDebug() << "Adding output to config" << output->id();
        }
        output->updateKScreenOutput(kscreenOutput);

        if (m_outputMap.count() == 1) {
            kscreenOutput->setPrimary(true);
        } else if (m_outputMap.count() > 1) {
            qWarning() << "Multiple outputs, but no way to figure out the primary one. :/";
        } else {
            qWarning() << "No outputs found. :(";
        }
        // FIXME: primaryScreen?
    }
}

QMap<quint32, WaylandOutput*> WaylandConfig::outputMap() const
{
    return m_outputMap;
}

