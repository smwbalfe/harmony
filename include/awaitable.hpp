//
// Created by shriller44 on 5/4/24.
//

#ifndef HARMONY_AWAITABLE_HPP
#define HARMONY_AWAITABLE_HPP
#include <memory>
#include <client.hpp>
#include <accept_data.hpp>
#include <request_data.hpp>
#include <socket_context.hpp>
#include <server_runtime.hpp>
#include <utility>

namespace harmony {
    struct accept_awaitable {
        std::shared_ptr<accept_data> acc_ptr_;
        explicit accept_awaitable(std::shared_ptr<accept_data> accept_data_ptr) :
                acc_ptr_(std::move(accept_data_ptr)) {


            acc_ptr_->entry_ = io_uring_get_sqe(&acc_ptr_->ring_.ring_);
            io_uring_prep_accept(acc_ptr_->entry_, acc_ptr_->server_fd_,
                                 (struct sockaddr *) &acc_ptr_->address_, &acc_ptr_->client_addr_len_, 0);
        }

        bool await_ready() { return false; }
        void await_suspend(
                std::coroutine_handle<> handle) noexcept {

            acc_ptr_->req_data_.handle_ = handle;
            acc_ptr_->req_data_.op_ = operation::accept;
            io_uring_sqe_set_data(acc_ptr_->entry_, &acc_ptr_->req_data_);
        }
        [[nodiscard]] client await_resume() {
            // We create a deep copy of the socket address pointer as we require it for
            // our endpoint
            return {.fd_ = acc_ptr_->req_data_.client_fd_,
                    .ep_ = create_endpoint(new sockaddr_storage_t(acc_ptr_->address_))};

        }
    };
    struct recv_awaitable {
        std::shared_ptr<recv_context> recv_ptr_;

        explicit recv_awaitable(std::shared_ptr<recv_context> recv_ptr) :
                recv_ptr_ {std::move(recv_ptr)} {
            recv_ptr_->entry_ = io_uring_get_sqe(&recv_ptr_->r_.ring_);
            io_uring_prep_recv( recv_ptr_->entry_ , recv_ptr_->client_fd_, recv_ptr_->buf_, recv_ptr_->buf_size_, 0);
        }

        bool await_ready() { return false; }

        void await_suspend(std::coroutine_handle<> handle) noexcept {

            recv_ptr_->req_data_.handle_ = handle;
            recv_ptr_->req_data_.op_ = operation::recv;
            io_uring_sqe_set_data(recv_ptr_->entry_, &recv_ptr_->req_data_);

        }

        [[nodiscard]] size_t await_resume() const {
            return recv_ptr_->req_data_.bytes_;
        }
    };
    struct send_awaitable {
        std::shared_ptr<send_context> send_ptr_;

        explicit send_awaitable(std::shared_ptr<send_context> send_ptr)
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
}
#endif //HARMONY_AWAITABLE_HPP
