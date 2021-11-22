#ifndef CHANNELRECORD_H
#define CHANNELRECORD_H


#include <QMap>
#include <QList>
#include <QString>
#include "main.h"

#define CHANNELRECORD_VER 2


struct ChannelRecord {
    quint32 version = CHANNELRECORD_VER;    // @ version 1
    double  freqAsMhz = 0.0;                // @ version 1; MHz
    qint32  freqAsKhz = 0;                       // @ version 2; kHz
    bool    showAtTray = false;             // @ version 1
    QString desc;                           // @ version 1

    ChannelRecord(qint32 freqAsKhz = 0, bool tray = false, const QString & desc = QString())
        : freqAsMhz(khz_to_mhz(freqAsKhz))
        , freqAsKhz(freqAsKhz)
        , showAtTray(tray)
        , desc(desc)
    {}

    static void registerTypes();
};
Q_DECLARE_METATYPE(ChannelRecord)

QDataStream &operator<<(QDataStream &out, const ChannelRecord &cr);
QDataStream &operator>>(QDataStream &in, ChannelRecord &cr);

typedef QList<ChannelRecord> ChannelRecordList;
Q_DECLARE_METATYPE(ChannelRecordList)

typedef QMap<qint32,ChannelRecord> ChannelRecordMap;
Q_DECLARE_METATYPE(ChannelRecordMap)

#endif // CHANNELRECORD_H
