//
// Created by shriller44 on 5/13/24.
//

#ifndef HARMONY_HTTP_REQUEST_HANDLER_HPP
#define HARMONY_HTTP_REQUEST_HANDLER_HPP

#include <unordered_map>
#include <string>

#include <task.hpp>
#include <http_parser.hpp>
#include <http_builder.hpp>
#include <client.hpp>
#include <filesystem>
#include <unordered_map>

namespace harmony {

    class http_request_handler {

    public:

        http_request_handler(){}

        void set_resource_dir(std::string_view resource_dir){
            resource_dir_ = resource_dir;
        }
        template<typename Handler>
        void get(std::string_view route, Handler handler){
            route_handler_[route] = handler;
        }

        harmony::task<harmony::response> execute(
                harmony::request& req){

            harmony::response res {resource_dir_};

            std::string_view resource = req.resource;
            if (route_handler_.contains(resource)){
                co_await route_handler_[resource](req, res);
                co_return res;
            } else {
                res.status_code = 404;
                co_return res;
            }
        }

    private:
        using router_handler_t = std::function<harmony::task<void>(harmony::request&, harmony::response&)>;
        std::unordered_map<std::string_view, router_handler_t> route_handler_;
        std::string_view resource_dir_;
    };
}

#endif //HARMONY_HTTP_REQUEST_HANDLER_HPP
