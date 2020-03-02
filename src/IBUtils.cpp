#include <iostream>
#include "IBUtils.h"
#include "HumanReadable.h"

uint32_t infiniband_radar::IBUtils::bitwise_highest(uint32_t value_a, uint32_t value_b) {
    uint32_t n = value_a & value_b;
    n |= (n >>  1);
    n |= (n >>  2);
    n |= (n >>  4);
    n |= (n >>  8);
    n |= (n >> 16);
    return n - (n >> 1);
}

void infiniband_radar::IBUtils::get_transfer_values(ib_portid_t *port_id, int port_num, uint64_t &xmit_bytes,
                                                     uint64_t &rcv_bytes, ibmad_port *ibmad_port, bool reset) {
    uint8_t pc[1024];
    memset(pc, 0, 1024);

    uint8_t resetCounter[1024];
    memset(resetCounter, 0, 1024);

    // TODO: Might be useful to check if the port actually has extended speed
    // TODO: WTF InfiniBand why is there no atomic IB_MAD_METHOD_GET & IB_MAD_METHOD_SET ?!
    if (!pma_query_via(pc, port_id, port_num, INFINIBANDRADAR_SMP_TIMEOUT, IB_GSI_PORT_COUNTERS_EXT, ibmad_port)) {
        std::cerr << "Failed query port: " << port_num << std::endl;
        return;
    }
    if (reset) {
        unsigned int mask = 0x0000;
        // TODO: Even more WTF.. Why do I have to reset IB_GSI_PORT_COUNTERS when I am querying IB_GSI_PORT_COUNTERS_EXT ?!?!
        performance_reset_via(resetCounter, port_id, port_num, mask, INFINIBANDRADAR_SMP_TIMEOUT, IB_GSI_PORT_COUNTERS, ibmad_port);
    }

    uint64_t val;
    // https://community.mellanox.com/docs/DOC-2572
    // "Total number of data octets, divided by 4 (lanes)"
    // TODO: What does lanes mean?

    mad_decode_field(pc, IB_PC_EXT_XMT_BYTES_F, &val);
    xmit_bytes = val * 4;

    mad_decode_field(pc, IB_PC_EXT_RCV_BYTES_F, &val);
    rcv_bytes = val * 4;
}

ib_portid_t infiniband_radar::IBUtils::get_queryable_port_id(ibnd_node_t *node) {
    ib_portid_t port_id = { 0 };

    if (node->type == IB_NODE_SWITCH) {
        ib_portid_set(&port_id, node->smalid, 0, 0);
    }
    else {
        for (int p = 1; p <= node->numports; p++) {
            if (node->ports[p]) {
                ib_portid_set(&port_id, node->ports[p]->base_lid, 0, 0);
                break;
            }
        }
    }

    return port_id;
}

const uint32_t infiniband_radar::IBUtils::get_highest_width(uint32_t width, uint32_t peer_width) {
    return bitwise_highest(width, peer_width);
}

const uint32_t infiniband_radar::IBUtils::get_highest_speed(uint32_t speed, uint32_t peer_speed) {
    return bitwise_highest(speed, peer_speed);
}

const uint32_t infiniband_radar::IBUtils::get_highest_speed_ext(uint32_t speed_ext, uint32_t peer_speed_ext) {
    return bitwise_highest(speed_ext, peer_speed_ext);
}

const uint32_t infiniband_radar::IBUtils::get_port_capability_mask(ibnd_port *port) {
    uint8_t* info = &port->info[0];
    if (port->node->type == IB_NODE_SWITCH) {
        if (port->node->ports[0])
            info = &port->node->ports[0]->info[0];
        else
            assert(port->node->ports[0]);
    }
    return mad_get_field(info, 0, IB_PORT_CAPMASK_F);
}

const bool infiniband_radar::IBUtils::is_port_capable_for_extended_speed(ibnd_port *port) {
    return static_cast<const bool>(get_port_capability_mask(port) & CL_NTOH32(IB_PORT_CAP_HAS_EXT_SPEEDS));
}

const std::string infiniband_radar::IBUtils::get_possible_port_speed(ibnd_port *port) {
    if(!port->remoteport) {
        return "???";
    }

    uint32_t own_width = mad_get_field(port->info, 0, IB_PORT_LINK_WIDTH_ENABLED_F);
    uint32_t own_speed = mad_get_field(port->info, 0, IB_PORT_LINK_SPEED_ENABLED_F);
    uint32_t own_speed_ext = 0;
    uint32_t own_fdr_10 = 0;

    if (is_port_capable_for_extended_speed(port)) {
        own_speed_ext = mad_get_field(port->info, 0, IB_PORT_LINK_SPEED_EXT_ENABLED_F);
        own_fdr_10 = mad_get_field(port->ext_info, 0, IB_MLNX_EXT_PORT_LINK_SPEED_ENABLED_F);
    }


    uint32_t peer_width = mad_get_field(port->remoteport->info, 0, IB_PORT_LINK_WIDTH_ENABLED_F);
    uint32_t peer_speed = mad_get_field(port->remoteport->info, 0, IB_PORT_LINK_SPEED_ENABLED_F);
    uint32_t peer_speed_ext = 0;
    uint32_t peer_fdr_10 = 0;

    if (is_port_capable_for_extended_speed(port->remoteport)) {
        peer_speed_ext = mad_get_field(port->remoteport->info, 0, IB_PORT_LINK_SPEED_EXT_ENABLED_F);
        peer_fdr_10 = mad_get_field(port->remoteport->ext_info, 0, IB_MLNX_EXT_PORT_LINK_SPEED_ENABLED_F);
    }

    uint32_t possible_width = get_highest_width(own_width, peer_width);
    uint32_t possible_speed = get_highest_speed(own_speed, peer_speed);
    uint32_t possible_speed_ext = get_highest_speed_ext(own_speed_ext, peer_speed_ext);
    uint32_t possible_fdr10 = own_fdr_10 & FDR10 && peer_fdr_10 & FDR10;

    return HumanReadable::link_type(possible_width, possible_speed, possible_speed_ext, possible_fdr10);
}

const std::string infiniband_radar::IBUtils::get_active_port_speed(ibnd_port *port) {
    uint32_t width = mad_get_field(port->info, 0, IB_PORT_LINK_WIDTH_ACTIVE_F);
    uint32_t speed = mad_get_field(port->info, 0, IB_PORT_LINK_SPEED_ACTIVE_F);

    uint32_t fdr_10 = 0;
    uint32_t extended_speed = 0;

    if (is_port_capable_for_extended_speed(port)) {
        extended_speed = mad_get_field(port->info, 0, IB_PORT_LINK_SPEED_EXT_ACTIVE_F);
        fdr_10 = mad_get_field(port->ext_info, 0, IB_MLNX_EXT_PORT_LINK_SPEED_ACTIVE_F);
    }

    return HumanReadable::link_type(width, speed, extended_speed, fdr_10);
}
