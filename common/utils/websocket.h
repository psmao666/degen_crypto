#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <functional>
#include <string>
#include <memory>
#include <queue>
#include <mutex>
#include "common/utils/logger.h"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

namespace degen_crypto { namespace common { namespace utils {

class WebSocketClient : public std::enable_shared_from_this<WebSocketClient> {
public:
    using MessageHandler = std::function<void(const std::string&)>;

    WebSocketClient(net::io_context& ioc, ssl::context& ctx)
        : resolver_(net::make_strand(ioc))
        , ws_(net::make_strand(ioc), ctx) {
    }
    ~WebSocketClient() {
        LOG_INFO(logger::g_logger, "WebSocketClient::closing websocket");
        close();
    }

    void connect(const std::string& host, const std::string& port, const std::string& target, MessageHandler handler) {
        host_ = host;
        target_ = target;
        message_handler_ = std::move(handler);

        LOG_INFO(logger::g_logger, "WebSocketClient::connect() connecting to {} on port {}", host, port);

        resolver_.async_resolve(host, port,
            beast::bind_front_handler(&WebSocketClient::on_resolve, shared_from_this()));
    }

    void send(const std::string& message) {
        std::lock_guard<std::mutex> lock(write_mutex_);
        bool write_in_progress = !write_queue_.empty();
        write_queue_.push(message);
        if (!write_in_progress) {
            do_write();
        }
    }

    void close() {
        ws_.async_close(websocket::close_code::normal,
            beast::bind_front_handler(&WebSocketClient::on_close, shared_from_this()));
    }

private:
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec) {
            LOG_ERROR(logger::g_logger, "Resolve failed: {}", ec.message());
            return;
        }

        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
        beast::get_lowest_layer(ws_).async_connect(results,
            beast::bind_front_handler(&WebSocketClient::on_connect, shared_from_this()));
    }

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
        if (ec) {
            LOG_ERROR(logger::g_logger, "Connect failed: {}", ec.message());
            return;
        }

        beast::get_lowest_layer(ws_).expires_never();
        ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

        if(!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
            ec = beast::error_code(static_cast<int>(::ERR_get_error()),
                net::error::get_ssl_category());
            LOG_ERROR(logger::g_logger, "SSL hostname failed: {}", ec.message());
            return;
        }

        ws_.next_layer().async_handshake(ssl::stream_base::client,
            beast::bind_front_handler(&WebSocketClient::on_ssl_handshake, shared_from_this()));
    }

    void on_ssl_handshake(beast::error_code ec) {
        if (ec) {
            LOG_ERROR(logger::g_logger, "SSL handshake failed: {}", ec.message());
            return;
        }

        ws_.async_handshake(host_, target_,
            beast::bind_front_handler(&WebSocketClient::on_handshake, shared_from_this()));
    }

    void on_handshake(beast::error_code ec) {
        if (ec) {
            LOG_ERROR(logger::g_logger, "Handshake failed: {}", ec.message());
            return;
        }

        do_read();
    }

    void do_read() {
        ws_.async_read(buffer_,
            beast::bind_front_handler(&WebSocketClient::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            if (ec != websocket::error::closed) {
                LOG_ERROR(logger::g_logger, "Read failed: {}", ec.message());
            }
            return;
        }

        if (message_handler_) {
            message_handler_(beast::buffers_to_string(buffer_.data()));
        }
        buffer_.consume(buffer_.size());
        do_read();
    }

    void do_write() {
        auto& msg = write_queue_.front();
        ws_.async_write(net::buffer(msg),
            beast::bind_front_handler(&WebSocketClient::on_write, shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t) {
        if (ec) {
            LOG_ERROR(logger::g_logger, "Write failed: {}", ec.message());
            return;
        }

        std::lock_guard<std::mutex> lock(write_mutex_);
        write_queue_.pop();
        if (!write_queue_.empty()) {
            do_write();
        }
    }

    void on_close(beast::error_code ec) {
        if (ec) {
            LOG_ERROR(logger::g_logger, "Close failed: {}", ec.message());
            return;
        }
    }

private:
    tcp::resolver resolver_;
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string target_;
    MessageHandler message_handler_;
    std::queue<std::string> write_queue_;
    std::mutex write_mutex_;
};

} // namespace utils
} // namespace common
} // namespace degen_crypto 