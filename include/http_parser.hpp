//
// Created by shriller44 on 12/8/23.
//

#ifndef SOCKETS_HTTP_PARSER_HPP
#define SOCKETS_HTTP_PARSER_HPP

#include <string>
#include <fmt/format.h>
#include <ranges>
#include <algorithm>
#include <vector>
#include <fstream>
#include <string>
#include <charconv>
#include <optional>
#include <coroutine>

#include <server_runtime.hpp>

namespace harmony {

    struct request {
        std::string method;
        std::string resource;
        std::string version;
        std::string host;
        std::string user_agent;
        uint32_t content_length = 0;
        std::string_view body;
    };

    class http_parser {
        public:

            http_parser() = default;
            http_parser(server_runtime* svr_rt): server_runtime_{svr_rt} {}

            /* reads the header, returns the buffer which may have read parts of the body*/
            harmony::task<std::tuple<std::string, std::string, int>> read_header(int client_fd){
                std::string request_buffer;
                char buf[4096];


                size_t header_end = 0;
                // TODO: validate the buffer starts with valid HTTP
                while(!harmony::http_parser::is_header_read(request_buffer, header_end)) {
                    request_buffer += std::string_view(buf, co_await server_runtime_->async_recv(client_fd, buf, 4096));
                    // fmt::print("{}\n", read_buf);
                }

                /*
                -  4 bytes = \r\n\r\n which is present between the last char of the header and first char of the body
                -   header_end = index of the \r of this 4 byte sequence so move forward 4 to be the start of body
                */
                co_return std::make_tuple(std::string{request_buffer.data(), header_end}, request_buffer, header_end + 4);
            };

            harmony::task<harmony::request> parse_http_request(int client_fd) {
                auto [header, buffer, body_start_offset] = co_await read_header(client_fd);
                harmony::request parsed_resquest = harmony::http_parser::parse_header(header);
                auto body_size = parsed_resquest.content_length;
                if (body_size != 0){
                    char read_buf[4096];
                    uint32_t body_bytes_read = buffer.size() - body_start_offset;
                    while (body_bytes_read != body_size) {
                        auto recv_bytes = co_await server_runtime_->async_recv(client_fd, read_buf, 4096);
                        buffer += std::string_view(read_buf, recv_bytes);
                        body_bytes_read = buffer.size() - body_start_offset;
                    }
                }
                parsed_resquest.body = std::string{buffer.begin() + body_start_offset,buffer.end()};
                co_return parsed_resquest;
            };
            bool is_crlf(const std::string_view check) {return (check[0] == '\r' and check[1] == '\n');}
            bool is_header_read(const std::string_view check_buf, size_t& pos){
                auto f = check_buf.find("\r\n\r\n");
                if (f != std::string_view::npos){
                    pos = f;
                    return true;
                }
                return false;
            }
            auto tokenize(const std::string_view str, const char delim) {
                std::vector<std::string_view> return_vec;
                for (const auto &item: std::views::split(str, delim)) {
                    return_vec.emplace_back(item);
                }
                return return_vec;
            }
            auto get_kv_pair(const std::string_view line) {
                auto first_colon = line.find(':');
                auto key = std::views::take(line, static_cast<int>(first_colon));
                auto value = std::string_view(line.begin() + first_colon + 2, line.end());
            
                return std::make_pair(key, value);
            }
            void scan_first_line(harmony::request &obj, const std::vector<std::string_view>& header_part) {
                auto tokens = tokenize(header_part.front(), ' ');

                obj.method = tokens[0];
                obj.resource = tokens[1];
                obj.version = tokens[2];
            }
            void debug_http(std::string_view h_string){
//                fmt::print("----- DEBUG HTTP ------\n{}\n------- END OF DEBUG HTTP ------\n", h_string);
            }
            harmony::request parse_header(std::span<char> http_request) {
                debug_http({http_request.begin(), http_request.end()});

                std::vector<std::string_view> header_parts;
                std::string_view body;
                bool reading_body = false;

                for (const auto &http_component: std::views::split(http_request, '\n')) {
                    auto sv_part = std::string_view(http_component);
                    sv_part.remove_suffix(1);
                    header_parts.emplace_back(sv_part);
                }

                harmony::request http;
                scan_first_line(http, header_parts);

                for (auto it = std::next(header_parts.begin()); it != header_parts.end(); ++it) {
                    auto [key, value] = get_kv_pair(*it);
                    if (key == "Content-Length") {
                        std::from_chars(value.data(), value.data() + value.size(), http.content_length);
                    }
//                    fmt::print("key: {} , value: '{}'\n", key, value);
                }
                return http;
            }
        private:
            harmony::server_runtime *server_runtime_;
    };
}
#endif //SOCKETS_HTTP_PARSER_HPP
