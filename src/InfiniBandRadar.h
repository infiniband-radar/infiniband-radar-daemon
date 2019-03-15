#ifndef INFINIBANDRADAR_INFINIBAND_RADAR_H
#define INFINIBANDRADAR_INFINIBAND_RADAR_H

#include <ibnetdisc.h>
#include <nlohmann/json.hpp>
#include "FabricDiscoveryHelper.h"

using nlohmann::json;

namespace infiniband_radar {
    class InfiniBandRadar {
    private:
        struct ibmad_port* _ibmad_port;
        FabricDiscoveryHelper _discovery_helper;

    public:
        InfiniBandRadar(const std::string &ca_name, int ca_port, const std::string& node_name_map_filename);
        InfiniBandRadar(InfiniBandRadar const &) = delete;
        void operator=(InfiniBandRadar const &) = delete;

        ~InfiniBandRadar();

        void write_port_stats(ibnd_node* node, std::shared_ptr<json> j, uint32_t expected_update_interval_sec);
        void write_fabric_topology(ibnd_node* node, std::shared_ptr<json> j);

        std::shared_ptr<json> update_fabric_topology();
        std::shared_ptr<json> get_port_stats(uint32_t expected_update_interval_sec);

        void reset_or_setup();
    };
}


#endif //INFINIBANDRADAR_INFINIBAND_RADAR_H
