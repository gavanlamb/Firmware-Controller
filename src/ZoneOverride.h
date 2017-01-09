#include  <cstdint>

#ifndef ZONE_OVERRIDE_H
#define ZONE_OVERRIDE_H

struct ZoneOverride{
    uint8_t overridePin;
    bool isLow;
    uint32_t lastLowReadTime;
    uint32_t readInterval;
};

#endif
