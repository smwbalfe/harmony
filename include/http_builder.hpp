//
// Created by shriller44 on 5/5/24.
//

#ifndef HARMONY_HTTP_BUILDER_HPP
#define HARMONY_HTTP_BUILDER_HPP

#include <fmt/format.h>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <task.hpp>
#include <http_parser.hpp>
#include <http_builder.hpp>
#include <client.hpp>
#include <filesystem>
#include <unordered_map>

namespace harmony{

    int needed_size = 0;
    int param_str_size = 0;
    int upper_bound = 150;
    std::string buffer;

    struct response {
        response(std::string_view resource_dir): resource_dir_{resource_dir}{}

        int status_code;
        std::string content_type;
        std::string version = "1.1";
        std::vector<char> body;
        void send_file(std::string_view file) {
            content_type = determine_content_type(file);

            std::optional<std::vector<char>> resource = load_resource(file);
            if (resource.has_value()) {
                body = resource.value();
                status_code = 200;
            } else{
                status_code = 404;
            }
        }

    private:
        std::string_view determine_content_type(std::string_view file){
            size_t dot_pos = file.find('.');
            std::string_view ext = file.substr(dot_pos+1);

            return mime_map_[ext];
        }

        std::optional<std::vector<char>> load_resource(std::string_view resource ){
                std::string resource_path = fmt::format("{}/{}", resource_dir_, resource);

                std::filesystem::path path = resource_path;
                if (!std::filesystem::exists(path)) {
                    fmt::print("{} does not exist", path.c_str());
                    return std::nullopt;
                }
                std::ifstream file(path, std::ios::binary | std::ios::ate);
                if (!file.is_open()) {
                    fmt::print("{} failed to open", path.c_str());
                    return std::nullopt;
                }
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                std::vector<char> file_buffer(size);
                if (!file.read(file_buffer.data(), size)) {
                    return std::nullopt;
                }
                return std::move(file_buffer);
        }
        std::string resource_dir_;
        std::unordered_map<std::string_view, std::string_view> mime_map_ {
                {"html", "text/html"}
        };
    };

    class http_response_builder {

    public:
        std::string to_string(harmony::response response) {

            std::string_view info;
            switch(response.status_code) {
                case 404: {
                    info = "Not Found";
                    break;
                }
                case 200: {
                    info = "Ok";
                    break;
                }
            }
            int body_size = response.body.size();
            std::string empty_body = response.body.empty() ? "" : "\r\n";

            int version_length = 3;
            int status_code_length = 3;
            size_t size_info = info.size();
            size_t content_type_size = response.content_type.size();
            size_t body_size_str_length = std::to_string(body_size).length();
            size_t empty_body_str_length = empty_body.size();
            size_t body_str_length = response.body.size();
            size_t new_param_size = version_length + status_code_length + size_info + content_type_size + body_size_str_length + empty_body_str_length + body_size_str_length;

            bool recalc;
            if (new_param_size > param_str_size){
                param_str_size = new_param_size;
                recalc = true;
            }
            else {
                recalc = false;
            }

            format_to(buffer, recalc , "HTTP/{} {} {}\r\nContent-Type: {}\r\nContent-Length: {} \r\n{}",
                      response.version,
                      response.status_code,
                      info,
                      response.content_type, body_size, empty_body);

            // starting from the end of the buffer insert the body
            buffer.insert(buffer.end(), response.body.begin(), response.body.end());
            return buffer;
        }

    private:
        // We fill in our buffer using the format string and args, we determine if we need to recalculate
        // based on the variations in lengths of the args
        template <typename... Args>
        constexpr std::size_t format_to(std::string &str, bool recalculate,
                                        fmt::format_string<Args...> fmt,
                                        Args &&...args) {
            if (recalculate) {
                // calculate how large the fmt string will be when replaced with the args.
                needed_size = formatted_size(fmt, args...);
            }

            // this perfectly resizes and overwrites the string to be the exact size and no more, if needed_size is larger
            str.resize_and_overwrite(needed_size, [&](char *s, std::size_t) {
                auto out_it = fmt::format_to(s, fmt, args...);

                // the end iterator - the start of the string = perfect size of the new string.
                return static_cast<std::size_t>(out_it - s);
            });

            return str.size();
        }
    };
}
#endif //HARMONY_HTTP_BUILDER_HPP
