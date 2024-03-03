//
// Created by shriller44 on 1/9/24.
//

#ifndef SOCKETS_ASYNC_SOCKET_HPP
#define SOCKETS_ASYNC_SOCKET_HPP

#include <netinet/in.h>
#include <liburing.h>
#include <cstring>
#include <fmt/format.h>
#include <deque>
#include <thread>
#include <utility>

#include "../async_runtime/io_uring.hpp"
#include "../async_runtime/io_context.hpp"


namespace harmony {

    struct recv_data {
        request_data req_data_;
        io_uring_sqe *entry_;
        char* buf_;
        size_t buf_size_;
        int client_fd_;
        iouring& r_;
    };

    struct send_data {
        request_data req_data_;
        io_uring_sqe *entry_;
        const char* buf_;
        size_t buf_size_;
        int client_fd_;
        iouring& r_;
    };

    struct recv_awaitable {
        std::shared_ptr<recv_data> recv_ptr_;

        explicit recv_awaitable(std::shared_ptr<recv_data> recv_ptr) :
            recv_ptr_ {std::move(recv_ptr)} {
            recv_ptr_->entry_ = io_uring_get_sqe(&recv_ptr_->r_.ring_);
            io_uring_prep_recv( recv_ptr_->entry_ , recv_ptr_->client_fd_, recv_ptr_->buf_, recv_ptr_->buf_size_, 0);
        }

        bool await_ready() { return false; }

        void await_suspend(std::coroutine_handle<> handle) noexcept {
            fmt::print("suspending awaitable (recv)\n");
            recv_ptr_->req_data_.handle_ = handle;
            recv_ptr_->req_data_.op_ = operation::recv;
            io_uring_sqe_set_data(recv_ptr_->entry_, &recv_ptr_->req_data_);
            fmt::print("awaitable suspended\n");
        }

        [[nodiscard]] size_t await_resume() const {
            return recv_ptr_->req_data_.bytes_;
        }
    };

    struct send_awaitable {
        std::shared_ptr<send_data> send_ptr_;

        explicit send_awaitable(std::shared_ptr<send_data> send_ptr)
        : send_ptr_ {std::move(send_ptr)} {
            send_ptr_->entry_ = io_uring_get_sqe(&send_ptr_->r_.ring_);
            io_uring_prep_send(send_ptr_->entry_, send_ptr_->client_fd_, send_ptr_->buf_, send_ptr_->buf_size_, 0);
        }
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> handle) noexcept {
            fmt::print("suspending awaitable (send)\n");
            send_ptr_->req_data_.handle_ = handle;
            send_ptr_->req_data_.op_ = operation::send;
            io_uring_sqe_set_data(send_ptr_->entry_, &send_ptr_->req_data_);
        }
        [[nodiscard]] size_t await_resume() const {
            return send_ptr_->req_data_.bytes_;
        }
    };

    struct socket {

        std::shared_ptr<recv_data> recv_ptr;
        std::shared_ptr<send_data> send_ptr;

        io_context& io_ctx_;
        explicit socket(io_context& io_ctx): io_ctx_ {io_ctx} {}

        recv_awaitable async_recv(int client_fd, char* buf, size_t size) {
            recv_ptr = std::make_shared<recv_data>(recv_data{
                .req_data_ = {},
                .entry_ = nullptr,
                .buf_ = buf,
                .buf_size_ = size,
                .client_fd_ = client_fd,
                 .r_ = io_ctx_.get_ring(),
            });
            return recv_awaitable{recv_ptr};
        }

        send_awaitable async_send(int client_fd, const char *send_buf, size_t size) {

            send_ptr = std::make_shared<send_data>(send_data{
                    .req_data_ = {},
                    .entry_ = nullptr,
                    .buf_ = send_buf,
                    .buf_size_ = size,
                    .client_fd_ = client_fd,
                    .r_ = io_ctx_.get_ring(),
            });

            return send_awaitable{send_ptr};
        }

    };
}

#endif //SOCKETS_ASYNC_SOCKET_HPP
