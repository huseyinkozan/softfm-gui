#ifndef CHANNELRECORD_H
#define CHANNELRECORD_H


#include <QMap>
#include <QList>
#include <QString>

struct ChannelRecord {
    quint32 version = 1;            // @ version 1
    double  freq = 0.0;             // @ version 1
    bool    showAtTray = false;     // @ version 1
    QString desc;                   // @ version 1

    ChannelRecord(double freq = 0.0, bool tray = false, const QString & desc = QString())
        : freq(freq)
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

typedef QMap<double,ChannelRecord> ChannelRecordMap;
Q_DECLARE_METATYPE(ChannelRecordMap)

#endif // CHANNELRECORD_H
