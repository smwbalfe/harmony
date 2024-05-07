//
// Created by shriller44 on 5/4/24.
//

#ifndef HARMONY_ACCEPT_DATA_HPP
#define HARMONY_ACCEPT_DATA_HPP
#include <netinet/in.h>
#include <liburing.h>
#include <cstring>
#include <fmt/format.h>
#include <deque>
#include <thread>
#include <utility>

#include "io_uring.hpp"

namespace harmony {
    struct accept_data {
        request_data req_data_;
        io_uring_sqe *entry_;
        sockaddr_storage_t address_;
        socklen_t client_addr_len_ = sizeof(sockaddr_storage_t);
        iouring& ring_;
        int server_fd_;
    };
}
#endif //HARMONY_ACCEPT_DATA_HPP
