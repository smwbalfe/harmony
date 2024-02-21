#include "../src/async_runtime/io_context.hpp"
#include "../src/server/async_client_acceptor.hpp"
#include "../src/server/async_socket.hpp"
#include "../src/http/http_parser.hpp"

#include <fstream>
#include <filesystem>

#include "simdjson.h"
using namespace simdjson;

const char* http_response = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html\r\n"
                            "Content-Length: 59\r\n"
                            "\r\n"
                            "<html><body><h1>Hello, this is an HTML response</h1></body></html";



int main()
{
    harmony::io_context io_ctx;
    harmony::acceptor acceptor;
    harmony::socket socket {io_ctx};

    /* reads the header, returns the buffer which may have read parts of the body*/
    auto read_header = [&](int client_fd) -> harmony::task<std::tuple<std::string, std::string, int>> {
        std::string request_buffer;
        char read_buf[4096];
        size_t header_end = 0;
        while(!harmony::http_parser::is_header_read(request_buffer, header_end)) {
            request_buffer += std::string_view(read_buf, co_await socket.async_recv(client_fd, read_buf));
        }

        /*
           -  4 bytes = \r\n\r\n which is present between the last char of the header and first char of the body
          -   header_end = index of the \r of this 4 byte sequence so move forward 4 to be the start of body
         */
        co_return std::make_tuple(std::string{request_buffer.data(), header_end}, request_buffer, header_end + 4);
    };

    auto read_and_parse_http_request = [&](int client_fd) -> harmony::task<harmony::parsed_http> {
        auto [header, buffer, body_start_offset] = co_await read_header(client_fd);
        harmony::http::request_header req_header = harmony::http_parser::parse_header(header);
        auto body_size = req_header.content_length;
        if (body_size != 0){
            char read_buf[4096];
            uint32_t body_bytes_read = buffer.size() - body_start_offset;
            while (body_bytes_read != body_size) {
                auto recv_bytes = co_await socket.async_recv(client_fd, read_buf);
                buffer += std::string_view(read_buf, recv_bytes);
                body_bytes_read = buffer.size() - body_start_offset;
            }
        }
        co_return harmony::parsed_http(req_header, std::string{buffer.begin() + body_start_offset,
                                                               buffer.end()});
    };

    auto find_resource = [&](std::string_view resource) -> harmony::task<std::optional<std::string>> {
        std::string file;
        if (resource == "/") {
            std::filesystem::path path = "/home/shriller44/dev/cpp/misc/poo-server/tests/index/index.html";
            if (!exists(path)) {
                co_return std::nullopt;
            }
            std::fstream f(path, std::ios::in);
            std::string line;
            while(std::getline(f, line)){
                file += line;
            }
        }
        co_return std::move(file);
    };

    auto write_response = [&](int client_fd, harmony::http::response response) -> harmony::task<void> {
        std::string data_to_send = response.to_string();
        uint32_t bytes_to_send = data_to_send.size();
        uint32_t sent_bytes = 0;
        while (bytes_to_send != sent_bytes){
            std::string_view subset {data_to_send.begin() + sent_bytes, data_to_send.end()};
            sent_bytes += co_await socket.async_send(client_fd, subset.data());
        }
    };

    auto client_handler = [&](harmony::client client) -> harmony::task<void> {

        auto [header, body] = co_await read_and_parse_http_request(client.fd_);

        auto resource = co_await find_resource(header.resource);

        harmony::http::response response;

        if (resource.has_value()){
            response.header.status_code = 200;
            response.header.content_type = "text/html";
            response.body = resource.value();
        } else {
            response.header.status_code = 404;
            response.body = "";
        }

        co_await write_response(client.fd_ , response);
    };

    auto server = [&]() -> harmony::task<void> {
        for (;;) {
            harmony::client client = co_await acceptor.async_accept(io_ctx);
            io_ctx.post_task(client_handler(std::move(client)));
        }
    };

    io_ctx.post_task(server());
    io_ctx.run();
    return 0;
}