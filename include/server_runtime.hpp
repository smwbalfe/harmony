//
// Created by shriller44 on 5/4/24.
//

#ifndef HARMONY_SERVER_RUNTIME_HPP
#define HARMONY_SERVER_RUNTIME_HPP

#include <cstdint>
#include <iostream>
#include <utility>

#include <task.hpp>
#include <awaitable.hpp>
#include <io_context.hpp>
#include <server_socket.hpp>

namespace harmony {
    class server_runtime {
        public:
            explicit server_runtime(int port): server_socket_ {port}{}
            accept_awaitable async_accept() {
                auto accept_context_ = std::make_shared<accept_data>(accept_data{
                        .req_data_ = {},
                        .entry_ = nullptr,
                        .address_ = {},
                        .ring_ = io_context_.ring(),
                        .server_fd_ = server_socket_.fd()
                });

                return accept_awaitable { accept_context_ };
            }
            recv_awaitable async_recv(int client_fd, char *buf, size_t size) {
                std::shared_ptr recv_ptr = std::make_shared<recv_context>(recv_context{
                        .req_data_ = {},
                        .entry_ = nullptr,
                        .buf_size_ = size,
                        .client_fd_ = client_fd,
                        .r_ = io_context_.ring(),
                        .buf_ = buf,
                });

                return recv_awaitable{recv_ptr};

            };
            send_awaitable async_send(int client_fd, const char *buf, size_t size) {
                std::shared_ptr recv_ptr = std::make_shared<send_context>(send_context{
                        .req_data_ = {},
                        .entry_ = nullptr,
                        .buf_size_ = size,
                        .client_fd_ = client_fd,
                        .r_ = io_context_.ring(),
                        .buf_ = buf,
                });
                return send_awaitable{recv_ptr};
            };
            template<typename T>
            void post_task(task<T> new_task) {io_context_.post_task(std::move(new_task));}
            void start(){
                server_socket_.start_listening();
                io_context_.run();
            }
        private:
            harmony::server_socket server_socket_;
            harmony::io_context io_context_;
    };
}

#endif //HARMONY_SERVER_RUNTIME_HPP
