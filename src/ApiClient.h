#ifndef INFINIBANDRADAR_APICLIENT_H
#define INFINIBANDRADAR_APICLIENT_H

#include <string>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

using nlohmann::json;

namespace infiniband_radar {
    /**
     * API client to send requests <br>
     * Multithreading of the same instance is not supported
     */
    class ApiClient {
    private:
        std::string _api_url;
        std::string _api_static_token;
        std::string _fabric_id;

        CURL* curl;
        curl_slist* headers = nullptr;

        void make_api_request(const std::string& path, const std::string& method, const std::shared_ptr<json> &payload);
    public:
        ApiClient(std::string api_host, std::string api_static_token, std::string fabric_id);
        ~ApiClient();
        ApiClient(ApiClient const &) = delete;
        void operator=(ApiClient const &) = delete;

        /**
         * Send a topology update request. Blocking call.
         * @param payload
         */
        void send_topology(std::shared_ptr<json> payload);

        /**
         * Send a new port statistic request. Blocking call.
         * @param payload
         */
        void send_port_stats(std::shared_ptr<json> payload);
    };
}


#endif //INFINIBANDRADAR_APICLIENT_H
