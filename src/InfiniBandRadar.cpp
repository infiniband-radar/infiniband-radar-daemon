#include <iostream>
#include "InfiniBandRadar.h"
#include "HumanReadable.h"
#include "IBUtils.h"

void infiniband_radar::InfiniBandRadar::write_port_stats(ibnd_node* node, std::shared_ptr<json> j, uint32_t expected_update_interval_sec) {

    ib_portid_t port_id = IBUtils::get_queryable_port_id(node);

    int start_port = 1;
    if (node->type == IB_NODE_SWITCH && node->smaenhsp0)
    {
        start_port = 0;
        ib_portid_set(&port_id, node->smalid, 0, 0);
    }

    for (int port_number = start_port; port_number <= node->numports; port_number++)
    {
        ibnd_port* port = node->ports[port_number];

        if (!port || !port->remoteport)
            continue; // Port is not connected to anything

        if (node->type != IB_NODE_SWITCH)
            ib_portid_set(&port_id, port->base_lid, 0, 0);

        uint64_t xmit;
        uint64_t rcv;

        IBUtils::get_transfer_values(&port_id, port_number, xmit, rcv, _ibmad_port, true);

        (*j)["metrics"].push_back({
                                     {"caGuid", HumanReadable::guid(port->node->guid)},
                                     {"portNumber", port->portnum},
                                     {"xmit", xmit/expected_update_interval_sec},
                                     {"rcv", rcv/expected_update_interval_sec},
                             });
    }
}

void infiniband_radar::InfiniBandRadar::write_fabric_topology(ibnd_node* node, std::shared_ptr<json> j) {
    json caObject = {
            {"guid", HumanReadable::guid(node->guid)},
            {"description", this->_discovery_helper.get_node_alias_name(node->guid, node->nodedesc)},
            {"type", HumanReadable::node_type(node)},
    };

    uint8_t sm_info[1024] = { 0 };
    //TODO use another _ibmad_port for those queries
    if(smp_query_via(sm_info, &node->path_portid, IB_ATTR_SMINFO, 0, INFINIBANDRADAR_SMP_TIMEOUT, _ibmad_port)) {
        uint32_t sm_state = mad_get_field(sm_info, 0, IB_SMINFO_STATE_F);
        uint32_t sm_priority = mad_get_field(sm_info, 0, IB_SMINFO_PRIO_F);
        caObject["subnetManager"] = {
                {"state", HumanReadable::sm_state(sm_state)},
                {"priority", sm_priority},
        };
    }

    (*j)["cas"].push_back(caObject);

    for (int portIdx = 1; portIdx <= node->numports; portIdx++) {
        ibnd_port_t *port = node->ports[portIdx];

        if(port == nullptr) {
            continue;
        }

        json portObject = {
                {"caGuid", HumanReadable::guid(port->node->guid)},
                {"portNumber", port->portnum},
        };

        if(port->remoteport != nullptr) {
            portObject["toCaGuid"] = HumanReadable::guid(port->remoteport->node->guid);
            portObject["toPortNumber"] = port->remoteport->portnum;
            portObject["speed"] = IBUtils::get_active_port_speed(port);
            portObject["possibleSpeed"] = IBUtils::get_possible_port_speed(port);
        }

        (*j)["ports"].push_back(portObject);
    }
}

infiniband_radar::InfiniBandRadar::InfiniBandRadar(const std::string &ca_name, int ca_port, const std::string& node_name_map_filename) : _discovery_helper(ca_name, ca_port, node_name_map_filename) {
    // TODO: remove unused classes
    int mgmt_classes[] = {
            IB_SMI_CLASS,
            IB_SMI_DIRECT_CLASS,
            IB_SA_CLASS,
            IB_PERFORMANCE_CLASS
    };

    _ibmad_port = mad_rpc_open_port(const_cast<char*>(ca_name.c_str()), ca_port, mgmt_classes, sizeof(mgmt_classes) / sizeof(mgmt_classes[0]));
    if(!_ibmad_port) {
        std::cerr << "Fail to open mad rpc port" << std::endl;
        exit(1);
    }

}

infiniband_radar::InfiniBandRadar::~InfiniBandRadar() {
    mad_rpc_close_port(_ibmad_port);
}

std::shared_ptr<json> infiniband_radar::InfiniBandRadar::update_fabric_topology() {
    _discovery_helper.update_node_map();

    auto topology_json = std::make_shared<json>();

    long update_fabric_ms = _discovery_helper.update_fabric();

    long write_fabric_topology_ms = _discovery_helper.foreach_node([&](ibnd_node* node) {
        write_fabric_topology(node, topology_json);
    });

    (*topology_json)["discoveryTimeMs"] = update_fabric_ms;
    (*topology_json)["writeJsonTimeMs"] = write_fabric_topology_ms;

    return topology_json;
}

std::shared_ptr<json> infiniband_radar::InfiniBandRadar::get_port_stats(uint32_t expected_update_interval_sec) {
    auto port_stats_json = std::make_shared<json>();

    long write_port_stats_ms = _discovery_helper.foreach_node([&](ibnd_node* node) {
        write_port_stats(node, port_stats_json, expected_update_interval_sec);
    });

    (*port_stats_json)["timeTookMs"] = write_port_stats_ms;

    return port_stats_json;
}

void infiniband_radar::InfiniBandRadar::reset_or_setup() {
    update_fabric_topology();

    // Reset all counters in the fabric
    auto throw_away = std::make_shared<json>();
    _discovery_helper.foreach_node([&](ibnd_node* node) {
        write_port_stats(node, throw_away, 1);
    });
}

