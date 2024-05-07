// Created by shriller44 on 12/8/23.
//

#ifndef HARMONY_SERVER_SOCKET_HPP
#define HARMONY_SERVER_SOCKET_HPP

#include <spdlog/spdlog.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <fmt/format.h>
#include <thread>
#include <memory>

#include "alias.hpp"
#include "endpoint.hpp"

namespace harmony {
    class server_socket {
    public:
        server_socket(int port, int backlog = 20) : port_(port), backlog_(backlog) {
            fd_ = bind_server_socket();
        }
        void start_listening() {
            if (listen(fd_, backlog_) == -1) {
                fmt::print(stderr, "listen error\n");
                std::exit(1);
            }
            spdlog::info("listening on port {}", port_);
        }
        ~server_socket() {
            close(fd_);
        }
        [[nodiscard]] int fd() const {return fd_;}
    private:
        int port_;
        int backlog_;
        int fd_;
        struct addr_info_deleter {
            void operator()(addrinfo *ai) {
                freeaddrinfo(ai);
            }
        };
        addrinfo *get_addr_info() {
            addrinfo hints;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET6;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;
            addrinfo *addrinfo;
            int rv = getaddrinfo(nullptr, std::to_string(port_).c_str(), &hints, &addrinfo);
            if (rv != 0) {
                fmt::print(stderr, "getaddrinfo error: {}\n", gai_strerror(rv));
                free(addrinfo);
                throw std::runtime_error("Failed to get address info");
            }
            return addrinfo;
        }
        int bind_socket(addrinfo *servinfo) {
            auto p = servinfo;
            for (; p != nullptr; p = p->ai_next) {
                fd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
                if (fd_ == -1) {
                    fmt::print(stderr, "server socket()");
                    continue;
                }
                int yes = 1;
                if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
                    fmt::print(stderr, "setsockopt()");
                    std::exit(1);
                }

                if (bind(fd_, p->ai_addr, p->ai_addrlen) == -1) {
                    close(fd_);
                    fmt::print(stderr, "server bind()");
                    continue;
                }
                break;
            }
            if (p == nullptr) {
                throw std::runtime_error("Failed to bind");
            } else {
                return fd_;
            }
        }
        int bind_server_socket() {
            std::unique_ptr<addrinfo, addr_info_deleter> addr_info_ptr{get_addr_info()};
            return bind_socket(addr_info_ptr.get());
        }
    };

} // namespace harmony

#endif // HARMONY_SERVER_SOCKET_HPP
