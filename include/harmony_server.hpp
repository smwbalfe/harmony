#include <cstdint>
#include <iostream>
#include <utility>

#include <task.hpp>
#include <http_parser.hpp>
#include <http_builder.hpp>
#include <client.hpp>
#include <filesystem>
#include <unordered_map>
#include <http_request_handler.hpp>

namespace harmony {

    class http_server {

        public:
            explicit http_server(uint16_t port):
                server_runtime_ {std::make_unique<harmony::server_runtime>(port)},
                http_parser_ {server_runtime_.get()}{}

            void start(){
                server_runtime_->post_task(client_acceptor());
                server_runtime_->start();
            }
            template<typename Handler>
            void get(std::string_view route, Handler handler){req_handler_.get(route, handler);}
            void set_resource_dir(std::string_view path){ req_handler_.set_resource_dir(path); }
        private:
            harmony::task<void> write_response(int client_fd, harmony::response response){
                harmony::http_response_builder resp_builder_;
                auto resp_string = resp_builder_.to_string(response);
                uint32_t bytes_to_send = resp_string.size();
                uint32_t sent_bytes = 0;
                while (bytes_to_send != sent_bytes){
                    std::string_view subset {resp_string.begin() + sent_bytes, resp_string.end()};
                    sent_bytes += co_await server_runtime_->async_send(client_fd, subset.data(), subset.size());
                }
            };

            harmony::task<void> client_handler(harmony::client client){
                for (;;) {
                    harmony::request request = co_await http_parser_.parse_http_request(client.fd_);
                    harmony::response response = co_await req_handler_.execute(request);
                    co_await write_response(client.fd_, response);
                }
            }

            harmony::task<void> client_acceptor() {
                for (;;) {
                    harmony::client client = co_await server_runtime_->async_accept();
                    spdlog::info("A new client has connected");
                    server_runtime_->post_task(client_handler(std::move(client)));
                }
            }
        private:
            std::string_view resource_dir_;
            std::unique_ptr<harmony::server_runtime> server_runtime_;
            harmony::http_parser http_parser_;
            harmony::http_request_handler req_handler_;
    };
}
