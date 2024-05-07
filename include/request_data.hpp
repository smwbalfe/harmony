//
// Created by shriller44 on 5/4/24.
//

#ifndef HARMONY_REQUEST_DATA_HPP
#define HARMONY_REQUEST_DATA_HPP
#include <coroutine>
#include "alias.hpp"

namespace harmony {
    enum class operation {
        accept, recv, send
    };

    struct request_data {
        std::coroutine_handle<> handle_;
        int client_fd_;
        operation op_;
        std::size_t bytes_;
        sockaddr_storage_t address_;
    };

    struct queue_item {
        std::coroutine_handle<> handle_;
    };
}

#endif //HARMONY_REQUEST_DATA_HPP
