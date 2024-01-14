//
// Created by shriller44 on 12/8/23.
//

#ifndef SOCKETS_IPV6_HPP
#define SOCKETS_IPV6_HPP

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory>
#include <fmt/format.h>
#include <arpa/inet.h>
#include <sstream>
#include <queue>
#include <optional>

// Self include headers
#include "endpoint.hpp"
#include "../alias.hpp"

namespace harmony {
    class ipv6 : public endpoint {
    public:
        explicit ipv6(sockaddr_in6_t *sin6) : sin6_{sin6} {}

        unsigned short port() override {
            return ntohs(sin6_->sin6_port);
        }

        int version() override { return 6; }

        std::string address() override {
            char addrStr[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &(sin6_->sin6_addr), addrStr, INET6_ADDRSTRLEN);
            return addrStr;
        }

        std::optional<std::string> hostname() override {
            char host[NI_MAXHOST];
            if (getnameinfo((sockaddr_t *) sin6_, sizeof(sockaddr_in6_t), host, NI_MAXHOST, nullptr, 0, 0) == 0) {
                return host;
            }
            return "Hostname not found";
        }

    private:
        sockaddr_in6_t *sin6_;
    };
}

#endif //SOCKETS_IPV6_HPP
