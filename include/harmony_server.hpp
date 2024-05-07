#include <cstdint>
#include <iostream>
#include <utility>

#include <task.hpp>
#include <http_parser.hpp>
#include <http_builder.hpp>
#include <client.hpp>
#include <filesystem>
#include <unordered_map>

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
            void get(std::string_view route, Handler handler){
                resource_map_[route] = handler;
            }

        private:
            harmony::task<std::optional<std::vector<char>>> find_resource(std::string_view resource, harmony::request req){

                std::string_view file_response {};
                if (resource == "/"){
                    co_await resource_map_[resource](req);
                    co_return std::nullopt;
                } else{
                    co_return std::nullopt;
                }

                std::string resource_path = fmt::format("{}/{}", resource_dir_, file_response);

                std::filesystem::path path = resource_path;
                if (!std::filesystem::exists(path)) {
                    fmt::print("{} does not exist", path.c_str());
                    co_return std::nullopt;
                }

                std::ifstream file(path, std::ios::binary | std::ios::ate);
                if (!file.is_open()) {
                    fmt::print("{} failed to open", path.c_str());
                    co_return std::nullopt;
                }

                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                std::vector<char> file_buffer(size);

                if (!file.read(file_buffer.data(), size)) {
                    co_return std::nullopt;
                }

                co_return std::move(file_buffer);
            };
            harmony::task<void> write_response(int client_fd, harmony::response response){
                std::string data_to_send = response.to_string();
                uint32_t bytes_to_send = data_to_send.size();
                uint32_t sent_bytes = 0;
                while (bytes_to_send != sent_bytes){
                    std::string_view subset {data_to_send.begin() + sent_bytes, data_to_send.end()};
                    sent_bytes += co_await server_runtime_->async_send(client_fd, subset.data(), subset.size());
                }
                fmt::print("total bytes sent: {}\n", sent_bytes);

            };
            harmony::task<void> client_handler(harmony::client client){
                for (;;) {
                    harmony::request request = co_await http_parser_.parse_http_request(client.fd_);
                    auto resource = co_await find_resource(request.resource, request);
                    harmony::response response;
                    if (resource.has_value()) {
                        response.header.status_code = 200;
                        response.header.content_type = "text/html";
                        response.body = resource.value();
                    } else {
                        response.header.status_code = 404;
                        response.header.content_type = "text/html";
                        std::string err = "<html><body><h1>404 not found</h1></body></html>";
                        response.body = std::vector<char>(err.begin(), err.end());
                    }
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
            std::string_view resource_dir_ = "/home/shriller44/dev/cpp/projects/harmony/data";
            std::unique_ptr<harmony::server_runtime> server_runtime_;
            harmony::http_parser http_parser_;
            std::unordered_map<std::string_view, std::function<harmony::task<void>(harmony::request)>> resource_map_;
    };
}
