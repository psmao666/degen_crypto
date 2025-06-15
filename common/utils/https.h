#pragma once

#include <utility>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <mutex>
#include <fmt/core.h>

namespace degen_crypto { namespace common { namespace utils {

class HttpsClient {
public:
    HttpsClient() : ioc_(), ctx_(boost::asio::ssl::context::tlsv12_client) {
        ctx_.set_verify_mode(boost::asio::ssl::verify_none);
    }

    template<typename ResponseType>
    std::optional<ResponseType> get(const std::string& host, const std::string& target) {
        std::lock_guard<std::mutex> lock(https_mutex_);
        
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
            
            boost::beast::http::write(stream, req);
            boost::beast::flat_buffer buffer;
            boost::beast::http::response<boost::beast::http::dynamic_body> res;
            boost::beast::http::read(stream, buffer, res);
            
            if (res.result() != boost::beast::http::status::ok) {
                return std::nullopt;
            }
            
            std::string response_body = boost::beast::buffers_to_string(res.body().data());
            auto json_response = nlohmann::json::parse(response_body);
            return json_response.get<ResponseType>();
            
        } catch(const std::exception& e) {
            return std::nullopt;
        }
    }

private:
    boost::asio::io_context ioc_;
    boost::asio::ssl::context ctx_;
    std::mutex https_mutex_;
};

} // namespace utils
} // namespace common
} // namespace degen_crypto 