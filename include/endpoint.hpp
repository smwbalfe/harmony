//
// Created by shriller44 on 5/4/24.
//

#ifndef HARMONY_ENDPOINT_HPP
#define HARMONY_ENDPOINT_HPP
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

#include "alias.hpp"

namespace harmony {

    class endpoint {
    public:
        virtual uint16_t port() = 0;

        virtual int version() = 0;

        virtual std::string address() = 0;

        virtual std::optional<std::string> hostname() = 0;

        virtual ~endpoint() = default;
    };
    class ipv4 : public endpoint {
    public:
        explicit ipv4(sockaddr_in *sin) : sin_{sin} {}

        uint16_t port() override {
            return ntohs(sin_->sin_port);
        }

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
    class ipv6 : public endpoint {
    public:
        explicit ipv6(sockaddr_in6_t *sin6) : sin6_{sin6} {}

        uint16_t port() override {
            auto val = ntohs(sin6_->sin6_port);
            return val;
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

    static inline std::unique_ptr<endpoint> create_endpoint(sockaddr_storage_t* storage) {

        auto sa = reinterpret_cast<struct sockaddr *>(&storage);
        if (sa->sa_family == AF_INET) {
            return std::make_unique<ipv4>(reinterpret_cast<sockaddr_in *>(sa));
        }
        return std::make_unique<ipv6>(reinterpret_cast<sockaddr_in6 *>(sa));
    }
}


#endif //HARMONY_ENDPOINT_HPP
