//
// Created by shriller44 on 1/8/24.
//

#ifndef SOCKETS_IOURING_HPP
#define SOCKETS_IOURING_HPP
#include <liburing.h>
#include <coroutine>
#include <deque>
#include "request_data.hpp"
#include <fmt/format.h>
#include <map>
#include <cerrno>
#include <cstring>


using io_uring_cqe = struct io_uring_cqe;
using io_uring = struct io_uring;
using io_uring_sqe = struct io_uring_sqe;
using sockaddr_in = struct sockaddr_in;



namespace harmony {

    class iouring {

    public:
        iouring() = default;

        explicit iouring(int queue_depth): ring_ {}, cqe_ { nullptr }
        {
            int result = io_uring_queue_init(queue_depth, &ring_, 0);
        }

        void submit_and_wait(std::deque<harmony::queue_item>& q){
            io_uring_submit_and_wait(&ring_, 1);
            process_cqe_entries(q);
        }

        void process_cqe_entries(std::deque<harmony::queue_item>& q){
            int processed {0};
            unsigned head;
            io_uring_for_each_cqe(&ring_, head, cqe_) {
                auto *req = static_cast<harmony::request_data *>(io_uring_cqe_get_data(cqe_));
                switch (req->op_){
                    case harmony::operation::accept: {
                        fmt::print("accepted operation\n");
                        req->client_fd_ = cqe_->res;
                        break;
                    }
                    case harmony::operation::recv: {
                    
                        req->bytes_ = cqe_->res;
                        break;
                    }
                    case harmony::operation::send: {
                        
                        req->bytes_ = cqe_->res;
                    }
                }
                q.emplace_back(harmony::queue_item{ req->handle_ });
                processed++;
            }
            io_uring_cq_advance(&ring_, processed);
        }

        io_uring ring_;
        io_uring_cqe *cqe_;
    };
}
#endif //SOCKETS_IOURING_HPP
