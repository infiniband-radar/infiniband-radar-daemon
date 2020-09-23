#include "FabricDiscoveryHelper.h"
#include <iostream>
#include <utility>
#include <fstream>
#include <sstream>

using infiniband_radar::FabricDiscoveryHelper;

infiniband_radar::FabricDiscoveryHelper::FabricDiscoveryHelper(std::string ca_name, int ca_port, const std::string& node_name_map_filename)
        : _ca_name(std::move(ca_name)),
          _ca_port(ca_port),
          _node_name_map_filename(node_name_map_filename)
{
}

infiniband_radar::FabricDiscoveryHelper::~FabricDiscoveryHelper() {
    _fabric_lock.lock();

    ibnd_destroy_fabric(_fabric);

    _fabric_lock.unlock();
}

long infiniband_radar::FabricDiscoveryHelper::update_fabric() {
    auto start = std::chrono::high_resolution_clock::now();

    struct ibnd_config config = {0};
    config.flags = IBND_CONFIG_MLX_EPI; // this flag allows to read Melanox's FDR10 property

    ibnd_fabric* new_fabric = ibnd_discover_fabric(const_cast<char*>(_ca_name.c_str()), _ca_port, nullptr, &config);

    if(new_fabric) {
        // Exchange the current fabric with the new fabric
        _fabric_lock.lock();

        ibnd_fabric* old_fabric = _fabric;
        _fabric = new_fabric;

        _fabric_lock.unlock();

        ibnd_destroy_fabric(old_fabric);

    }
    else {
        std::cerr << "Fail to discover fabric" << std::endl;
        //TODO: Do something if fail
    }

    auto finish = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(finish-start).count();

}

long infiniband_radar::FabricDiscoveryHelper::foreach_node(std::function<void(ibnd_node*)> function) {
    assert(_fabric != nullptr);

    auto start = std::chrono::high_resolution_clock::now();

    _fabric_lock.lock();

    ibnd_node* current_node = _fabric->nodes;

    do {
        function(current_node);
    }
    while((current_node = current_node->next) != nullptr);

    _fabric_lock.unlock();
    auto finish = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(finish-start).count();
}


const std::string& infiniband_radar::FabricDiscoveryHelper::get_node_alias_name(uint64_t guid, const std::string& original_description) const {
    auto itr = _node_name_map.find(guid);
    if (itr == _node_name_map.end()) {
        return original_description;
    }
    return itr->second;
}


void infiniband_radar::FabricDiscoveryHelper::update_node_map() {
    if(_node_name_map_filename.empty())
        return;

    std::ifstream i(_node_name_map_filename);
    if(i.fail()) {
        std::cerr << "Fail to open node_name_map '" << _node_name_map_filename << "'" << std::endl;
        return;
    }

    _node_name_map.clear();
    std::string line;
    while(std::getline(i, line)) {
        uint64_t guid;
        const char* currentChar = line.c_str();

        while(isspace(*currentChar))
            currentChar++;

        if (*currentChar == '\0' || *currentChar == '\n' || *currentChar == '#')
            continue; // Next line

        char* endChar;
        guid = strtoull(currentChar, &endChar, 0);
        if (endChar == currentChar || (!isspace(*endChar) && *endChar != '#' && *endChar != '\0')) {
            std::cerr << "Fail to parse node_name_map: Unexpected line ending" << std::endl;
            return;
        }
        currentChar = endChar;

        while(isspace(*currentChar))
            currentChar++;

        if(*currentChar != '"') {
            std::cerr << "Fail to parse node_name_map: Unexpected '\"'" << std::endl;
            return;
        }
        currentChar++;
        std::stringstream aliasName;

        while(*currentChar != '"') {
            aliasName << *currentChar;
            currentChar++;
        }

        _node_name_map[guid] = aliasName.str();
    }
}
