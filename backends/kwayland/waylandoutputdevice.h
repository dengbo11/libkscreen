/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef WAYLANDOUTPUTDEVICE_H
#define WAYLANDOUTPUTDEVICE_H

#include "waylandoutputdevicemode.h"

#include "qwayland-kde-output-device-v2.h"

#include "kscreen_export.h"
#include "types.h"

#include <QPoint>
#include <QSize>

namespace KScreen
{
class WaylandOutputConfiguration;

class WaylandOutputDevice : public QObject, public QtWayland::kde_output_device_v2
{
    Q_OBJECT

public:
    WaylandOutputDevice(int id);

    ~WaylandOutputDevice();

    QByteArray edid() const;
    bool enabled() const;
    int id() const;
    QString name() const;
    QString model() const;
    QString manufacturer() const;
    qreal scale() const;
    QPoint globalPosition() const;
    QSize pixelSize() const;
    int refreshRate() const;
    uint32_t vrrPolicy() const;
    uint32_t overscan() const;
    uint32_t capabilities() const;
    uint32_t rgbRange() const;

    OutputPtr toKScreenOutput();
    void updateKScreenOutput(OutputPtr &output);
    void updateKScreenModes(OutputPtr &output);

    bool isPrimary() const;
    void setPrimary(bool primary);
    void setIndex(uint32_t priority);
    uint32_t index() const;
    bool setWlConfig(WaylandOutputConfiguration *wlConfig, const KScreen::OutputPtr &output);

    QString modeId() const;
    QString uuid() const
    {
        return m_uuid;
    }

Q_SIGNALS:
    void done();

protected:
    void kde_output_device_v2_geometry(int32_t x,
                                       int32_t y,
                                       int32_t physical_width,
                                       int32_t physical_height,
                                       int32_t subpixel,
                                       const QString &make,
                                       const QString &model,
                                       int32_t transform) override;
    void kde_output_device_v2_current_mode(struct ::kde_output_device_mode_v2 *mode) override;
    void kde_output_device_v2_mode(struct ::kde_output_device_mode_v2 *mode) override;
    void kde_output_device_v2_done() override;
    void kde_output_device_v2_scale(wl_fixed_t factor) override;
    void kde_output_device_v2_edid(const QString &raw) override;
    void kde_output_device_v2_enabled(int32_t enabled) override;
    void kde_output_device_v2_uuid(const QString &uuid) override;
    void kde_output_device_v2_serial_number(const QString &serialNumber) override;
    void kde_output_device_v2_eisa_id(const QString &eisaId) override;
    void kde_output_device_v2_capabilities(uint32_t flags) override;
    void kde_output_device_v2_overscan(uint32_t overscan) override;
    void kde_output_device_v2_vrr_policy(uint32_t vrr_policy) override;
    void kde_output_device_v2_rgb_range(uint32_t rgb_range) override;
    void kde_output_device_v2_name(const QString &name) override;
    void kde_output_device_v2_high_dynamic_range(uint32_t hdr_enabled) override;
    void kde_output_device_v2_sdr_brightness(uint32_t sdr_brightness) override;
    void kde_output_device_v2_wide_color_gamut(uint32_t wcg_enabled) override;

private:
    QString modeName(const WaylandOutputDeviceMode *m) const;
    WaylandOutputDeviceMode *deviceModeFromId(const int modeId) const;

    WaylandOutputDeviceMode *m_mode;
    QList<WaylandOutputDeviceMode *> m_modes;

    int m_id;
    QPoint m_pos;
    QSize m_physicalSize;
    int32_t m_subpixel;
    QString m_manufacturer;
    QString m_model;
    int32_t m_transform;
    qreal m_factor;
    QByteArray m_edid;
    int32_t m_enabled;
    QString m_uuid;
    QString m_serialNumber;
    QString m_outputName;
    QString m_eisaId;
    uint32_t m_flags;
    uint32_t m_overscan;
    uint32_t m_vrr_policy;
    uint32_t m_rgbRange;
    uint32_t m_index;
    bool m_hdrEnabled = false;
    uint32_t m_sdrBrightness = 200;
    bool m_wideColorGamutEnabled = false;
};

}

KSCREEN_EXPORT QDebug operator<<(QDebug dbg, const KScreen::WaylandOutputDevice *output);

#endif // WAYLANDOUTPUTDEVICE_H
