#include "ApiClient.h"
#include <utility>
#include <iostream>
#include <chrono>

class ApiClientCurlGlobalInitAndCleanupHandler {
public:
    ApiClientCurlGlobalInitAndCleanupHandler() {
        CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);

        if (code) {
            std::cerr << "[ERROR] curl_global_init: " << code << std::endl;
        }
    }

    ~ApiClientCurlGlobalInitAndCleanupHandler() {
        curl_global_cleanup();
    }
};

/**
 * Will be constructed at the start of the program
 * and deconstructed at the end of the program
 */
static ApiClientCurlGlobalInitAndCleanupHandler initAndCleanupHandler;


infiniband_radar::ApiClient::ApiClient(std::string api_entry_point, std::string api_static_token, std::string fabric_id) {
    _api_url = std::move(api_entry_point);
    _api_static_token = std::move(api_static_token);
    _fabric_id = std::move(fabric_id);

    curl = curl_easy_init();
    assert(curl);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    headers = curl_slist_append(headers, ("Authorization: StaticToken " + _api_static_token).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

}

infiniband_radar::ApiClient::~ApiClient() {
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

void infiniband_radar::ApiClient::send_topology(std::shared_ptr<json> payload) {
    make_api_request("/v2/topologies/" + _fabric_id, "PUT", payload);
}

void infiniband_radar::ApiClient::send_port_stats(std::shared_ptr<json> payload) {
    make_api_request("/v2/metrics/" + _fabric_id, "PUT", payload);
}

size_t my_dummy_write(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    return size * nmemb;
}

void infiniband_radar::ApiClient::make_api_request(const std::string &path, const std::string &method, const std::shared_ptr<json> &payload) {
    auto total_start = std::chrono::high_resolution_clock::now();
    auto json_start = std::chrono::high_resolution_clock::now();
    std::string payload_string = payload->dump();
    auto json_end = std::chrono::high_resolution_clock::now();

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, (_api_url + path).c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_string.c_str());
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &my_dummy_write);

    auto perform_start = std::chrono::high_resolution_clock::now();
    CURLcode res = curl_easy_perform(curl);
    auto perform_end = std::chrono::high_resolution_clock::now();

    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    assert(res == CURLE_OK);

    auto total_end = std::chrono::high_resolution_clock::now();

    long json_time = std::chrono::duration_cast<std::chrono::milliseconds>(json_end-json_start).count();
    long perform_time = std::chrono::duration_cast<std::chrono::milliseconds>(perform_end-perform_start).count();
    long total_time = std::chrono::duration_cast<std::chrono::milliseconds>(total_end-total_start).count();

    std::cout << "HTTP Request: '" << path << "' "
                << "(j/p/total)"
                << json_time << "/" << perform_time << "/" << total_time
                << " ms" << std::endl;
}
