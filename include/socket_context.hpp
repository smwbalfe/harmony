//
// Created by shriller44 on 5/5/24.
//

#ifndef HARMONY_SOCKET_CONTEXT_HPP
#define HARMONY_SOCKET_CONTEXT_HPP

#include <cstdint>
#include <iostream>
#include <utility>
#include <liburing.h>
#include <request_data.hpp>
#include "io_uring.hpp"

namespace harmony {

    struct recv_context {
        request_data req_data_;
        io_uring_sqe *entry_;
        size_t buf_size_;
        int client_fd_;
        iouring &r_;
        char *buf_;
    };

    struct send_context {
        request_data req_data_;
        io_uring_sqe *entry_;
        size_t buf_size_;
        int client_fd_;
        iouring &r_;
        const char *buf_;
    };
}
#endif //HARMONY_SOCKET_CONTEXT_HPP
