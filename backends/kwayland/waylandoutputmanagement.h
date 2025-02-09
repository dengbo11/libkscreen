/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef WAYLANDOUTPUTMANAGEMENT_H
#define WAYLANDOUTPUTMANAGEMENT_H

#include "qwayland-kde-output-management-v2.h"
#include "qwayland-kde-output-order-v1.h"

#include <QObject>
#include <QSize>

namespace KScreen
{
class WaylandConfig;
class WaylandOutputDevice;

class WaylandOutputConfiguration : public QObject, public QtWayland::kde_output_configuration_v2
{
    Q_OBJECT
public:
    WaylandOutputConfiguration(struct ::kde_output_configuration_v2 *object);

Q_SIGNALS:
    void applied();
    void failed();

protected:
    void kde_output_configuration_v2_applied() override;
    void kde_output_configuration_v2_failed() override;
};

class WaylandOutputManagement : public QObject, public QtWayland::kde_output_management_v2
{
    Q_OBJECT
public:
    WaylandOutputManagement(struct ::wl_registry *registry, int id, int version);

    WaylandOutputConfiguration *createConfiguration();
};

class WaylandOutputOrder : public QObject, public QtWayland::kde_output_order_v1
{
    Q_OBJECT
public:
    WaylandOutputOrder(struct ::wl_registry *registry, int id, int version);
    ~WaylandOutputOrder();

    QVector<QString> order() const;

Q_SIGNALS:
    void outputOrderChanged(const QVector<QString> &outputs);

private:
    void kde_output_order_v1_output(const QString &output_name) override;
    void kde_output_order_v1_done() override;

    QVector<QString> m_outputOrder;
    QVector<QString> m_pendingOutputOrder;
};
}

#endif // WAYLANDOUTPUTMANAGEMENT_H
