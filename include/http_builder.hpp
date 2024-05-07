//
// Created by shriller44 on 5/5/24.
//

#ifndef HARMONY_HTTP_BUILDER_HPP
#define HARMONY_HTTP_BUILDER_HPP

#include <fmt/format.h>
#include <vector>

namespace harmony{
    int needed_size = 0;
    int param_str_size = 0;
    int upper_bound = 150;

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

        fmt::print("size needed: {}\n", needed_size);

        // this perfectly resizes and overwrites the string to be the exact size and no more, if needed_size is larger
        str.resize_and_overwrite(needed_size, [&](char *s, std::size_t) {
            auto out_it = fmt::format_to(s, fmt, args...);

            // the end iterator - the start of the string = perfect size of the new string.
            return static_cast<std::size_t>(out_it - s);
        });

        return str.size();
    }

    struct response_header {
        int status_code;
        std::string content_type;
        std::string version = "1.1";
    };


    std::string buffer;

    struct response {

        response_header header;
        std::vector<char> body;
        constexpr std::string to_string() {
            std::string_view info;
            switch(header.status_code) {
                case 404: {
                    info = "Not Found";
                    break;
                }
                case 200: {
                    info = "Ok";
                    break;
                }
            }
            int body_size = body.size();
            std::string empty_body = body.empty() ? "" : "\r\n";

            int version = 3;
            int status_code = 3;
            int size_info = info.size();
            int content_type_size = header.content_type.size();
            int body_size_str_length = std::to_string(body_size).length();
            int empty_body_str_length = empty_body.size();
            int body_str_length = body.size();

            int new_param_size = version + status_code + size_info + content_type_size + body_size_str_length + empty_body_str_length + body_size_str_length;
            fmt::print("new param: {} | param_str: {}", new_param_size, param_str_size);

            bool recalc;
            if (new_param_size > param_str_size){
                param_str_size = new_param_size;
                recalc = true;
            }
            else {
                recalc = false;
            }

            auto string_size = format_to(buffer,recalc , "HTTP/{} {} {}\r\nContent-Type: {}\r\nContent-Length: {} \r\n{}",
                                         header.version, header.status_code, info, header.content_type, body_size, empty_body);


            buffer.insert(buffer.end(), body.begin(), body.end());
            return buffer;
        }
    };

}
#endif //HARMONY_HTTP_BUILDER_HPP
