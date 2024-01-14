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

namespace harmony {


    namespace http{

            struct request_header {
                std::string method;
                std::string resource;
                std::string version;
                std::string host;
                std::string user_agent;
                uint32_t content_length = 0;
            };

    }

    namespace http_parser {
        struct request {
            std::string_view method;
            std::string_view resource;
            std::string_view version;
            std::string_view host;
            std::string_view user_agent;
            uint32_t content_length;
        };

        bool is_crlf(const std::string_view check) {
            return (check[0] == '\r' and check[1] == '\n');
        }

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
            auto value = std::string_view(line.begin() + first_colon + 1, line.end());
            value.remove_prefix(1);
            return std::make_pair(key, value);
        }

        void scan_first_line(http::request_header &obj, const std::vector<std::string_view>& header_part) {
            auto tokens = tokenize(header_part.front(), ' ');

            obj.method = tokens[0];
            obj.resource = tokens[1];
            obj.version = tokens[2];
        }

        void debug_http(std::string_view h_string){
            fmt::print("----- DEBUG HTTP ------\n{}\n------- END OF DEBUG HTTP ------\n", h_string);
        }

        http::request_header parse_header(std::span<char> http_request) {
            debug_http({http_request.begin(), http_request.end()});

            std::vector<std::string_view> header_parts;
            std::string_view body;
            bool reading_body = false;

            for (const auto &http_component: std::views::split(http_request, '\n')) {
                auto sv_part = std::string_view(http_component);
                sv_part.remove_suffix(1);
                header_parts.emplace_back(sv_part);
            }

            http::request_header http;
            scan_first_line(http, header_parts);

            for (const auto &hp: std::views::take(std::views::reverse(header_parts),
                                                  static_cast<int>(header_parts.size()) - 1)) {
                auto [key, value] = get_kv_pair(hp);
                if (key == "Content-Length") {
                    std::from_chars(value.data(), value.data() + value.size(), http.content_length);
                }
            }

            fmt::print("content length: {}\n", http.content_length);

            return http;
        }

    }

    struct parsed_http {
        harmony::http::request_header header;
        std::string_view body;
    };

}
#endif //SOCKETS_HTTP_PARSER_HPP
