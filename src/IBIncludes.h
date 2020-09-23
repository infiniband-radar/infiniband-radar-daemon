#ifndef INFINIBANDRADAR_F_H
#define INFINIBANDRADAR_F_H

// InfiniBand stuff
// #include <iba/ib_types.h>
#include <mad.h>
#include <ibnetdisc.h>

// Common stuff
#include <cstdint>

// Defining custom constants for 2x and HDR because there are not defined in ib_types.h
// TODO: Check if they are included in newer versions
#define IB_LINK_WIDTH_ACTIVE_2X 16 // 2x
#define IB_LINK_SPEED_EXT_ACTIVE_50 4 // HDR

#endif // INFINIBANDRADAR_F_H
