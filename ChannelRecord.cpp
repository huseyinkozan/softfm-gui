#include "ChannelRecord.h"

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
    out << cr.version << cr.freq << cr.showAtTray << cr.desc;
    return out;
}

QDataStream &operator>>(QDataStream &in, ChannelRecord &cr)
{
    in >> cr.version;
    if (cr.version == 1) {
        in >> cr.freq >> cr.showAtTray >> cr.desc;
    }
    return in;
}
