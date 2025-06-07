#pragma once
#define ASIO_STANDALONE

#include <asio.hpp>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

namespace asio_net {

    using asio::ip::tcp;

    class ServerConnection {
    public:
        ServerConnection(asio::io_context& io, uint16_t port)
            : acceptor_(io, tcp::endpoint(tcp::v4(), port)), socket_(io) {}

        tcp::socket& accept() {
            acceptor_.accept(socket_);
            return socket_;
        }

        static void send(tcp::socket& socket, const std::vector<char>& data) {
            std::lock_guard<std::mutex> lock(io_mutex_);
            int len = (int)data.size();
            asio::write(socket, asio::buffer(&len, sizeof(len)));
            asio::write(socket, asio::buffer(data));
        }

        static std::vector<char> recv(tcp::socket& socket) {
            std::lock_guard<std::mutex> lock(io_mutex_);
            int len = 0;
            asio::read(socket, asio::buffer(&len, sizeof(len)));
            std::vector<char> buf(len);
            asio::read(socket, asio::buffer(buf));
            return buf;
        }
        tcp::socket accept_socket() {
            tcp::socket socket(acceptor_.get_executor());
            acceptor_.accept(socket);
            return socket;
        }
    private:
        tcp::acceptor acceptor_;
        tcp::socket socket_;
        static inline std::mutex io_mutex_;
    };

    class ClientConnection {
    public:
        ClientConnection(asio::io_context& io, const std::string& host, uint16_t port)
            : socket_(io) {
            tcp::resolver resolver(io);
            auto endpoints = resolver.resolve(host, std::to_string(port));
            asio::connect(socket_, endpoints);
        }

        void send(const std::vector<char>& data) {
            int len = (int)data.size();
            asio::write(socket_, asio::buffer(&len, sizeof(len)));
            asio::write(socket_, asio::buffer(data));
        }

        std::vector<char> recv() {
            int len = 0;
            asio::read(socket_, asio::buffer(&len, sizeof(len)));
            std::vector<char> buf(len);
            asio::read(socket_, asio::buffer(buf));
            return buf;
        }

        tcp::socket& socket() { return socket_; }

    private:
        tcp::socket socket_;
    };

}  // namespace asio_net
