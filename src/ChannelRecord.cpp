#include "ChannelRecord.h"

#include <QDebug>


void ChannelRecord::registerTypes()
{
    qRegisterMetaType<ChannelRecord>("ChannelRecord");
    qRegisterMetaTypeStreamOperators<ChannelRecord>("ChannelRecord");

    qRegisterMetaType<ChannelRecordMap>("ChannelRecordMap");
    qRegisterMetaTypeStreamOperators<ChannelRecordMap>("ChannelRecordMap");

    qRegisterMetaType<ChannelRecordList>("ChannelRecordList");
    qRegisterMetaTypeStreamOperators<ChannelRecordList>("ChannelRecordList");
}

QDataStream &operator<<(QDataStream &out, const ChannelRecord &cr)
{
    out << cr.version;

    if (cr.version == 1) {
        double freqAsMhz = khz_to_mhz(cr.freqAsKhz);
        out << freqAsMhz;
    }
    if (cr.version == 2) {
        out << cr.freqAsKhz;
    }

    out << cr.showAtTray << cr.desc;

    return out;
}

QDataStream &operator>>(QDataStream &in, ChannelRecord &cr)
{
    in >> cr.version;

    if (cr.version == 1) {
        in >> cr.freqAsMhz;
        cr.freqAsKhz = mhz_to_khz(cr.freqAsMhz);
    }
    if (cr.version == 2) {
        in >> cr.freqAsKhz;
        cr.freqAsMhz = khz_to_mhz(cr.freqAsKhz);
    }

    in >> cr.showAtTray >> cr.desc;

    return in;
}
