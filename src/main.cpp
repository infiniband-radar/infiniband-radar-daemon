#include <nlohmann/json.hpp>
#include <iostream>
#include <csignal>
#include <future>
#include "InfiniBandRadar.h"
#include "ApiClient.h"
#include <thread>
#include <fstream>

using nlohmann::json;
using infiniband_radar::InfiniBandRadar;
using infiniband_radar::ApiClient;

static std::promise<void> exit_flag;
static auto exit_flag_future = exit_flag.get_future();

// check for std::make_unique, which is C++14 specific, and provide a local
// implementation for C++11
#ifdef __cpp_lib_make_unique
#include <memory>
namespace lib {
    using std::make_unique;
}
#else
namespace lib {
    template<typename T, typename... TArgs>
    std::unique_ptr<T> make_unique(TArgs&&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<TArgs>(args)...));
    }
}
#endif

std::unique_ptr<std::thread> set_interval(const std::function<void()> &function, long ms, const std::string& desc) {
    return lib::make_unique<std::thread>([function, ms, desc]() {
        auto chronoMs = std::chrono::milliseconds(ms);
        auto chronoLastIterationTook = std::chrono::milliseconds(0);

        // If the exit flag was set, we want to exit the loop
        while (exit_flag_future.wait_for(chronoMs - chronoLastIterationTook) == std::future_status::timeout) {
            auto start = std::chrono::high_resolution_clock::now();
            function();
            auto finish = std::chrono::high_resolution_clock::now();
            chronoLastIterationTook = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
            long tookms = chronoLastIterationTook.count();
            std::cout << desc << ": " << tookms << "ms" << std::endl;
        }
    });
}

void signal_handler(int signal) {
    std::cout << "Received signal: " << signal << std::endl;
    exit_flag.set_value();
    std::cout << "Exit flag was set" << std::endl;
}

void start(const std::string &api_entry_point,
           const std::string &api_static_token,
           const std::string &fabric_id,
           const std::string &ca_name,
           int ca_port,
           uint32_t port_stats_interval_sec,
           uint32_t topology_update_interval_sec,
           const std::string &node_name_map_filename
) {
    InfiniBandRadar radar(ca_name, ca_port, node_name_map_filename);

    radar.reset_or_setup();

    {
        ApiClient api(api_entry_point, api_static_token, fabric_id);

        std::cout << "Sending initial topology" << std::endl;
        auto initial_topology = radar.update_fabric_topology();
        api.send_topology(initial_topology);
    }

    /*
     * Timing stuff
     */
    const auto topology_update_interval = std::chrono::seconds(topology_update_interval_sec);
    const auto port_stats_interval = std::chrono::seconds(port_stats_interval_sec);
    const auto port_stats_interval_in_sec = port_stats_interval.count();

    auto port_stats_thread = set_interval([&]() {
        ApiClient api(api_entry_point, api_static_token, fabric_id);
        // TODO: Might need to give get_port_stats another parameter,
        // since there could be the case that the query took longer then expected
        auto port_stats = radar.get_port_stats(port_stats_interval_in_sec);
        api.send_port_stats(port_stats);
    }, std::chrono::duration_cast<std::chrono::milliseconds>(port_stats_interval).count(), "Send port stats");

    auto topology_thread = set_interval([&]() {
        ApiClient api(api_entry_point, api_static_token, fabric_id);
        auto topology = radar.update_fabric_topology();
        api.send_topology(topology);
    }, std::chrono::duration_cast<std::chrono::milliseconds>(topology_update_interval).count(), "Send topology");

    topology_thread->join();
    port_stats_thread->join();

    std::cout << "Stopped" << std::endl;
}

int main(int argc, char** argv) {
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);

    // TODO: remove those warnings if a mad_rpc request timed out
    // madrpc_show_errors(false);

    if(argc != 2) {
        std::cout << "Usage: " << argv[0] << " <Config File>" << std::endl;
        return 1;
    }

    std::string api_entry_point;
    std::string api_token;
    std::string fabric_id;

    std::string ca_name;
    int ca_port;


    std::string node_name_map_filename = "";

    uint32_t topology_update_interval_sec;
    uint32_t port_stats_interval_sec;

    try {
        std::ifstream i(argv[1]);
        if(i.fail()) {
            std::cerr << "Fail to open file '" << argv[1] << "'" << std::endl;
            return 1;
        }
        json j;
        i >> j;
        i.close();

        api_entry_point = j["api"]["entry_point"].get<std::string>();
        api_token = j["api"]["token"].get<std::string>();
        fabric_id = j["api"]["fabric_id"].get<std::string>();

        ca_name = j["ca_name"].get<std::string>();
        ca_port = j["ca_port"].get<int>();

        topology_update_interval_sec = j["topology_update_interval_sec"].get<uint32_t>();
        port_stats_interval_sec = j["port_stats_interval_sec"].get<uint32_t>();

        if(j["node_name_map"].is_string()) {
            node_name_map_filename = j["node_name_map"].get<std::string>();
        }
    }
    catch(const std::exception& e) {
        std::cerr << "Fail to parse config" << std::endl;
        return 1;
    };

    start(api_entry_point, api_token, fabric_id, ca_name, ca_port, port_stats_interval_sec, topology_update_interval_sec, node_name_map_filename);

    return 0;
}
