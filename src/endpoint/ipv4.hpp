//
// Created by shriller44 on 12/8/23.
//

#ifndef SOCKETS_IPV4_HPP
#define SOCKETS_IPV4_HPP
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

// Local headers
#include "endpoint.hpp"
#include "../alias.hpp"

namespace harmony {
    class ipv4 : public endpoint {
    public:
        explicit ipv4(sockaddr_in *sin) : sin_{sin} {}

        uint16_t port() override { return ntohs(sin_->sin_port); }
        int version() override { return 4; }
        std::string address() override {
            char addrStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(sin_->sin_addr), addrStr, INET_ADDRSTRLEN);
            return addrStr;
        }
        std::optional<std::string> hostname() override {
            char host[NI_MAXHOST];
            if (getnameinfo((sockaddr_t *) sin_, sizeof(sockaddr_in_t), host, NI_MAXHOST, nullptr, 0, 0) == 0) {
                return host;
            }
            return "";
        }

    private:
        sockaddr_in *sin_;
    };
}
#endif //SOCKETS_IPV4_HPP
