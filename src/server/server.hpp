//
// Created by shriller44 on 12/8/23.
//

#ifndef SOCKETS_SERVER_HPP
#define SOCKETS_SERVER_HPP
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

#include "../alias.hpp"
#include "../endpoint/ipv6.hpp"
#include "../endpoint/ipv4.hpp"
#include "../../temp/client_acceptor.hpp"
#include "../http/http_builder.hpp"

namespace harmony {
    struct addr_info_deleter {
        void operator()(addrinfo_t *ai) {
            freeaddrinfo(ai);
        }
    };
    addrinfo_t *get_addr_info(int port) {
        addrinfo_t hints;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        addrinfo_t *addrinfo;
        int rv = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &addrinfo);
        if (rv != 0) {
            fmt::print(stderr, "getaddrinfo error: {}\n", gai_strerror(rv));
            free(addrinfo);
        }
        return addrinfo;
    }
    int bind_socket(addrinfo_t *servinfo) {
        int server_fd = 0;
        auto p = servinfo;
        for (; p != nullptr; p = p->ai_next) {
            if ((server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                fmt::print(stderr, "server socket()");
                continue;
            }
            int yes = 1;
            if (setsockopt(static_cast<int>(server_fd), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
                fmt::print(stderr, "setsockopt()");
                std::exit(1);
            }

            if (bind(static_cast<int>(server_fd), p->ai_addr, p->ai_addrlen) == -1) {
                close(static_cast<int>(server_fd));
                fmt::print(stderr, "server bind()");
                continue;
            }
            break;
        }
        if (p == nullptr) {
            fmt::print("Failed to bind");
            exit(1);
        } else {
            return server_fd;
        }
    }
    int create_server(int port, int backlog = 20) {
        std::unique_ptr<addrinfo, addr_info_deleter> addr_info_ptr {get_addr_info(port)} ;
        auto server_fd = bind_socket(addr_info_ptr.get());
        if (listen(static_cast<int>(server_fd), static_cast<int>(backlog)) == -1) {
            fmt::print("listen error\n");
            exit(1);
        }
        fmt::print("started serve\n");
        return server_fd;
    }
}
#endif //SOCKETS_SERVER_HPP
