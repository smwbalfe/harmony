#include <fmt/format.h>
#include <harmony_server.hpp>

int main() {
    harmony::http_server server {8081};

    server.set_resource_dir("../data");

    server.get("/", [](harmony::request& request, harmony::response& response) -> harmony::task<void> {
        fmt::print("index route hit\n");
        co_return response.send_file("index.html");
    });

    server.get("/burger", [](harmony::request& request, harmony::response& response) -> harmony::task<void> {
        fmt::print("burger route hit\n");
        co_return response.send_file("burger.html");
    });

    server.start();
}
