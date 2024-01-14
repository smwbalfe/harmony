//
// Created by shriller44 on 12/8/23.
//

#ifndef SOCKETS_HTTP_BUILDER_HPP
#define SOCKETS_HTTP_BUILDER_HPP

#include <fmt/format.h>



namespace harmony::http {

//    template<typename... Args>
//    size_t format_to( std::string & str, std::format_string<Args...> fmt, Args &&... args ) {
//        auto const needed_size = formatted_size( fmt, args... );
//        str.resize_and_overwrite( needed_size, [&]( char * s, std::size_t ) {
//            auto out_it = format_to( s, fmt, args... );
//            return static_cast<std::size_t>( out_it - s );
//        } );
//        return str.size();
//    }

    struct response_header {
        int status_code;
        std::string content_type;
        std::string version = "1.1";
    };


    struct response {
        response_header header;
        std::string body;

        // refactor - calculate the size dynamically to reserve a formatter properly
        // refactor - have a pre-allocated string to be used each time that is resized each time.
        std::string to_string() {
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

            // calculate the size dynamically.
//            static thread_local auto buffer = std::string{};
//            static constexpr std::string_view fmt = "....";
//            auto const needed_size = formatted_size( fmt, params... );
//            buffer.resize_and_overwrite( needed_size, []( char * s, std::size_t ) {
//                auto out_it = format_to( s, fmt, params... );
//                return static_cast<std::size_t>( out_it - s );
//            } );

            std::string buffer;
            auto string_size = format_to(buffer, "HTTP/{} {} {}\r\nContent-Type: {}\r\nContent-Length: {} \r\n",
                                         header.version, header.status_code,info, "text/html", body.size());

            return buffer;


            // consider streaming  a large body
            return fmt::format("HTTP/{} {} {}\r\nContent-Type: {}\r\nContent-Length: {}\r\n{}{}",
                           header.version, // initial line
                           header.status_code, // initial line
                           info, // initial line
                           "text/html",  // Content-Type
                           body.size(),  // Content-Length
                           body.empty() ? "" : "\r\n" // body \r\n or "" if no body
            ,body); // body, if empty = "";
        }
    };

}


#endif //SOCKETS_HTTP_BUILDER_HPP
