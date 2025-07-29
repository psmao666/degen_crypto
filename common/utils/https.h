#pragma once

#include <utility>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <mutex>
#include <iostream>
#include <fmt/core.h>
#include "common/utils/logger.h"
#include "common/utils/time.h"
#include "common/utils/signature.h"

namespace degen_crypto { namespace common { namespace utils {
using namespace degen_crypto::logger;

class HttpsClient {
public:
    HttpsClient() : ioc_(), ctx_(boost::asio::ssl::context::tlsv12_client) {
        ctx_.set_verify_mode(boost::asio::ssl::verify_none);
    }

public:
    template<typename ResponseType>
    std::optional<ResponseType> get(const std::string& host, const std::string& target, const std::string& api_secret, const std::unordered_map<std::string, std::string>& headers = {}, bool need_signature = false, int n_retries = 3) {
        if (!need_signature) return get_helper<ResponseType>(host, target, headers, n_retries);
        
        auto timestamp = common::utils::get_current_ms_epoch(); 
        std::string query_string = fmt::format("timestamp={}", timestamp);
        std::string signature = common::utils::generate_signature(query_string, api_secret);
        query_string += fmt::format("&signature={}", signature);
        query_string = fmt::format("{}?{}", target, query_string);
        
        return get_helper<ResponseType>(host, query_string, headers, n_retries);
    }
    // New overload for POST with body
    template<typename ResponseType>
    std::optional<ResponseType> post(const std::string& host, const std::string& target, const std::string& body, const std::string& api_secret, const std::unordered_map<std::string, std::string>& headers = {}, bool need_signature = false, int n_retries = 3) {
        if (!need_signature) return post_helper<ResponseType>(host, target, body, headers, n_retries);
        
        auto timestamp = common::utils::get_current_ms_epoch(); 
        std::string query_string = fmt::format("timestamp={}", timestamp);
        std::string signature = common::utils::generate_signature(query_string, api_secret);
        query_string += fmt::format("&signature={}", signature);
        std::string new_target = fmt::format("{}?{}", target, query_string);
        return post_helper<ResponseType>(host, new_target, body, headers, n_retries);
    }

private:
    template<typename ResponseType>
    std::optional<ResponseType> get_helper(const std::string& host, const std::string& target, const std::unordered_map<std::string, std::string>& headers = {}, int n_retires = 3) {
        std::lock_guard<std::mutex> lock(https_mutex_);
        for (int attempt = 0; attempt < n_retires; ++attempt) {
            try {
                boost::asio::ip::tcp::resolver resolver(ioc_);
                const auto& results = resolver.resolve(host, "https");
                boost::beast::ssl_stream<boost::beast::tcp_stream> stream(ioc_, ctx_);
                if(!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
                    throw boost::beast::system_error(
                        boost::beast::error_code(
                            static_cast<int>(::ERR_get_error()),
                            boost::asio::error::get_ssl_category()),
                        "Failed to set SNI Hostname");
                }
                boost::beast::get_lowest_layer(stream).connect(results);
                stream.handshake(boost::asio::ssl::stream_base::client);
                boost::beast::http::request<boost::beast::http::string_body> req{
                    boost::beast::http::verb::get, target, 11
                };
                req.set(boost::beast::http::field::host, host);
                req.set(boost::beast::http::field::user_agent, "DegenCrypto/1.0");
                req.set(boost::beast::http::field::accept, "application/json");
                req.set(boost::beast::http::field::connection, "close");
                for (const auto& [key, value] : headers) {
                    req.set(key, value);
                }
                boost::beast::http::write(stream, req);
                boost::beast::flat_buffer buffer;
                boost::beast::http::response<boost::beast::http::dynamic_body> res;
                boost::beast::http::read(stream, buffer, res);
                if (res.result() != boost::beast::http::status::ok) {
                    LOG_ERROR(g_logger, "HTTP Error: {} with payload: {}, trying again..", boost::beast::buffers_to_string(res.body().data()), target);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
                std::string response_body = boost::beast::buffers_to_string(res.body().data());
                auto json_response = nlohmann::json::parse(response_body);
                return json_response.get<ResponseType>();
            } catch(const std::exception& e) {
                if (attempt == 2) return std::nullopt;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        return std::nullopt;
    }

    // Overload with body
    template<typename ResponseType>
    std::optional<ResponseType> post_helper(const std::string& host, const std::string& target, const std::string& body, const std::unordered_map<std::string, std::string>& headers = {}, int n_retries = 3) {
        std::lock_guard<std::mutex> lock(https_mutex_);
        for (int attempt = 0; attempt < n_retries; ++attempt) {
            try {
                boost::asio::ip::tcp::resolver resolver(ioc_);
                const auto& results = resolver.resolve(host, "https");
                boost::beast::ssl_stream<boost::beast::tcp_stream> stream(ioc_, ctx_);
                if(!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
                    throw boost::beast::system_error(
                        boost::beast::error_code(
                            static_cast<int>(::ERR_get_error()),
                            boost::asio::error::get_ssl_category()),
                        "Failed to set SNI Hostname");
                }
                boost::beast::get_lowest_layer(stream).connect(results);
                stream.handshake(boost::asio::ssl::stream_base::client);
                boost::beast::http::request<boost::beast::http::string_body> req{
                    boost::beast::http::verb::post, target, 11
                };
                req.set(boost::beast::http::field::host, host);
                req.set(boost::beast::http::field::user_agent, "DegenCrypto/1.0");
                req.set(boost::beast::http::field::accept, "application/json");
                req.set(boost::beast::http::field::connection, "close");
                req.set(boost::beast::http::field::content_type, "application/x-www-form-urlencoded");
                for (const auto& [key, value] : headers) {
                    req.set(key, value);
                }
                req.body() = body;
                req.prepare_payload();
                boost::beast::http::write(stream, req);
                boost::beast::flat_buffer buffer;
                boost::beast::http::response<boost::beast::http::dynamic_body> res;
                boost::beast::http::read(stream, buffer, res);
                if (res.result() != boost::beast::http::status::ok) {
                    LOG_ERROR(g_logger, "HTTP Error: {} with payload: {}", boost::beast::buffers_to_string(res.body().data()), target);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
                std::string response_body = boost::beast::buffers_to_string(res.body().data());
                auto json_response = nlohmann::json::parse(response_body);
                return json_response.get<ResponseType>();
            } catch(const std::exception& e) {
                if (attempt == 2) return std::nullopt;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        return std::nullopt;
    }

private:
    boost::asio::io_context ioc_;
    boost::asio::ssl::context ctx_;
    std::mutex https_mutex_;
};

} // namespace utils
} // namespace common
} // namespace degen_crypto 