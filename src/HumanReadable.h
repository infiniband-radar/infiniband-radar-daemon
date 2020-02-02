#ifndef INFINIBANDRADAR_TOHUMANREADABLE_H
#define INFINIBANDRADAR_TOHUMANREADABLE_H

#include "IBIncludes.h"
#include <string>
#include <iomanip>

namespace infiniband_radar {
    /**
     * Static class to convert some kind of arbitrary value to a human readable string
     */
    class HumanReadable {
    public:
        /**
         * No initialization should be possible
         */
        HumanReadable() = delete;


        static const std::string guid(uint64_t guid) {
            std::stringstream stream;
            stream << "0x"
                   << std::setfill('0')
                   << std::setw(sizeof(uint64_t) * 2)
                   << std::hex << guid;

            return stream.str();
        }

        static const std::string sm_state(uint32_t state) {
            switch (state) {
                case IB_SMINFO_STATE_NOTACTIVE:
                    return "NotActive";
                case IB_SMINFO_STATE_DISCOVERING:
                    return "Discovering";
                case IB_SMINFO_STATE_STANDBY:
                    return "Standby";
                case IB_SMINFO_STATE_MASTER:
                    return "Master";
                default:
                    return "??";
            }
        }

        static const std::string link_width(uint32_t width) {
            switch (width) {
                case IB_LINK_WIDTH_ACTIVE_1X:
                    return "1x";
                case IB_LINK_WIDTH_ACTIVE_4X:
                    return "4x";
                case IB_LINK_WIDTH_ACTIVE_8X:
                    return "8x";
                case IB_LINK_WIDTH_ACTIVE_12X:
                    return "12x";
                default:
                    return "??";
            }
        }

        static const std::string link_speed(uint32_t speed, uint32_t extended_speed, uint32_t fdr_10) {
            switch (speed) {
                case IB_LINK_SPEED_ACTIVE_2_5:
                    return "SDR";
                case IB_LINK_SPEED_ACTIVE_5:
                    return "DDR";
                case IB_LINK_SPEED_ACTIVE_10:
                    if (fdr_10 & FDR10)
                        return "FDR10";
                    if (!extended_speed) {
                        return "QDR";
                    }
                    switch (extended_speed) {
                        case IB_LINK_SPEED_EXT_ACTIVE_14:
                            return "FDR";
                        case IB_LINK_SPEED_EXT_ACTIVE_25:
                            return "EDR";
                        default:
                            break;
                    }
                default:
                    return "???";
            }
        }

        static const std::string link_type(uint32_t width, uint32_t speed, uint32_t extended_speed, uint32_t fdr_10) {
            return link_width(width) + link_speed(speed, extended_speed, fdr_10);
        }

        static const std::string node_type(ibnd_node* node) {
            switch (node->type) {
                case IB_NODE_SWITCH:
                    return "SW";//Switch
                case IB_NODE_CA:
                    return "CA";//Channel Adapter
                case IB_NODE_ROUTER:
                    return "RT";//Router
                default:
                    return "??";
            }
        }
    };
}

#endif //INFINIBANDRADAR_TOHUMANREADABLE_H
