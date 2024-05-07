#include <fmt/format.h>
#include <harmony_server.hpp>

int main() {
    harmony::http_server server {8081};

    server.get("/", [](harmony::request request) -> harmony::task<void> {
        fmt::print("index route hit\n");
        co_return;
    });

    server.start();
}
