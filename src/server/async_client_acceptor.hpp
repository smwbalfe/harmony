//
// Created by shriller44 on 12/27/23.
//

#ifndef SOCKETS_ASYNC_CLIENT_ACCEPTOR_HPP
#define SOCKETS_ASYNC_CLIENT_ACCEPTOR_HPP

#include <memory>
#include <utility>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../endpoint/endpoint.hpp"
#include "../endpoint/ipv4.hpp"
#include "../endpoint/ipv6.hpp"
#include "../alias.hpp"
#include "../async_runtime/io_context.hpp"

namespace harmony {

    struct client {
        int fd_;
        std::unique_ptr<endpoint> ep_;
    };

    std::unique_ptr<endpoint> create_endpoint(sockaddr_storage_t* storage) {

        auto sa = reinterpret_cast<struct sockaddr *>(storage);
        if (sa->sa_family == AF_INET) {
            return std::make_unique<ipv4>(reinterpret_cast<sockaddr_in *>(sa));
        }
        return std::make_unique<ipv6>(reinterpret_cast<sockaddr_in6 *>(sa));
    }

    struct accept_data {
        request_data req_data_;
        io_uring_sqe *entry_;
        sockaddr_storage_t address_;
        socklen_t client_addr_len_ = sizeof(sockaddr_storage_t);
        iouring& r_;
    };

    struct accept_awaitable {
        std::shared_ptr<accept_data> acc_ptr_;
        explicit accept_awaitable( std::shared_ptr<accept_data> accept_data_ptr) :
                acc_ptr_(std::move(accept_data_ptr)) {


            acc_ptr_->entry_ = io_uring_get_sqe(&acc_ptr_->r_.ring_);
            io_uring_prep_accept(acc_ptr_->entry_, acc_ptr_->r_.server_socket_,
                                 (struct sockaddr *) &acc_ptr_->address_, &acc_ptr_->client_addr_len_, 0);
        }

        bool await_ready() { return false; }
        void await_suspend(
                std::coroutine_handle<> handle) noexcept {
            fmt::print("suspending awaitable (accept)\n");
            acc_ptr_->req_data_.handle_ = handle;
            acc_ptr_->req_data_.op_ = operation::accept;
            io_uring_sqe_set_data(acc_ptr_->entry_, &acc_ptr_->req_data_);
        }
        [[nodiscard]] client await_resume() {
            return {.fd_ = acc_ptr_->req_data_.client_fd_,
                    .ep_ = create_endpoint(&acc_ptr_->address_)};
        }
    };

    struct acceptor {
        accept_awaitable async_accept(io_context& ioc) {

            acc_ptr_ = std::make_shared<accept_data>(accept_data{
                    .req_data_ = {},
                    .entry_ = nullptr,
                    .address_ = {},
                    .r_ = ioc.get_ring()
            });

            return accept_awaitable { acc_ptr_ };
        }

        std::shared_ptr<accept_data> acc_ptr_;
    };

}
#endif //SOCKETS_ASYNC_CLIENT_ACCEPTOR_HPP
