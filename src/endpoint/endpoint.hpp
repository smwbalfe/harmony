//
// Created by shriller44 on 12/8/23.
//

#ifndef SOCKETS_ENDPOINT_HPP
#define SOCKETS_ENDPOINT_HPP

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

namespace harmony {

    class endpoint {
    public:
            virtual uint16_t port() = 0;
            virtual int version() = 0;
            virtual std::string address() = 0;
            virtual std::optional<std::string> hostname() = 0;
            virtual ~endpoint() = default;
    };
}

#endif //SOCKETS_ENDPOINT_HPP
