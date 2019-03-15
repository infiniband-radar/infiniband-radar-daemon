#ifndef INFINIBANDRADAR_INFINIBANDUTILS_H
#define INFINIBANDRADAR_INFINIBANDUTILS_H

#include "IBIncludes.h"
#include <string>

#define INFINIBANDRADAR_SMP_TIMEOUT 50 /*ms*/

namespace infiniband_radar {
    class IBUtils {
    private:
        static const uint32_t bitwise_highest(uint32_t value_a, uint32_t value_b);

    public:
        /**
         * No initialization should be possible
         */
        IBUtils() = delete;

        static void get_transfer_values(ib_portid_t* port_id, int port_num, uint64_t& xmit_bytes, uint64_t& rcv_bytes, ibmad_port* ibmad_port, bool reset);

        static ib_portid_t get_queryable_port_id(ibnd_node_t* node);

        static const uint32_t get_highest_width(uint32_t width, uint32_t peer_width);

        static const uint32_t get_highest_speed(uint32_t speed, uint32_t peer_speed);

        static const uint32_t get_highest_speed_ext(uint32_t speed_ext, uint32_t peer_speed_ext);

        static const uint32_t get_port_capability_mask(ibnd_port* port);

        static const bool is_port_capable_for_extended_speed(ibnd_port* port);

        static const std::string get_possible_port_speed(ibnd_port* port);;

        static const std::string get_active_port_speed(ibnd_port* port);;
    };
}

#endif //INFINIBANDRADAR_INFINIBANDUTILS_H
