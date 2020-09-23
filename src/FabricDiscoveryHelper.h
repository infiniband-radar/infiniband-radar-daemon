#ifndef INFINIBANDRADAR_FABRICDISCOVERYHELPER_H
#define INFINIBANDRADAR_FABRICDISCOVERYHELPER_H

#include "IBIncludes.h"
#include <string>
#include <mutex>
#include <functional>
#include <map>

namespace infiniband_radar
{
    class FabricDiscoveryHelper {
    private:
        ibnd_fabric* _fabric = nullptr;
        std::mutex _fabric_lock;

        const std::string _ca_name;
        const int _ca_port;

        const std::string _node_name_map_filename;
        std::map<long, std::string> _node_name_map;

    public:
        FabricDiscoveryHelper(std::string ca_name, int ca_port, const std::string& node_name_map_filename);
        FabricDiscoveryHelper(FabricDiscoveryHelper const &) = delete;
        void operator=(FabricDiscoveryHelper const &) = delete;

        ~FabricDiscoveryHelper();

        long update_fabric();
        long foreach_node(std::function<void(ibnd_node*)> function);

        void update_node_map();

        const std::string& get_node_alias_name(uint64_t guid, const std::string& original_description) const;
    };
}



#endif //INFINIBANDRADAR_FABRICDISCOVERYHELPER_H
