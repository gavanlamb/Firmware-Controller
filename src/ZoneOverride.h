#include  <cstdint>

#ifndef ZONE_OVERRIDE_H
#define ZONE_OVERRIDE_H

struct ZoneOverride{
    uint8_t overridePin;
    uint32_t lastRead;
    uint32_t readInterval;
};

#endif
