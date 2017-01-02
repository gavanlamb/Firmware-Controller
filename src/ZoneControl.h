#include  <cstdint>

#ifndef ZONE_CONTROL_H
#define ZONE_CONTROL_H

struct ZoneControl{
    uint8_t zone;
    uint32_t duration;
    uint32_t startTime;
    uint32_t endTime;
};

#endif
