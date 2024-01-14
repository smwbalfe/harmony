//
// Created by shriller44 on 12/8/23.
//

#ifndef SOCKETS_ALIAS_HPP
#define SOCKETS_ALIAS_HPP

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>


namespace harmony {
    using addrinfo_t = struct addrinfo;
    using sockaddr_storage_t = struct sockaddr_storage;
    using sockaddr_t = struct sockaddr;
    using sockaddr_in_t = struct sockaddr_in;
    using sockaddr_in6_t = struct sockaddr_in6;
}

#endif //SOCKETS_ALIAS_HPP
