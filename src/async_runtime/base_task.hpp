//
// Created by shriller44 on 1/8/24.
//

#ifndef SOCKETS_BASE_TASK_HPP
#define SOCKETS_BASE_TASK_HPP

#include <coroutine>
#include <vector>
#include <fmt/format.h>
namespace harmony {
    struct base {
        struct promise_type;
        using handle = std::coroutine_handle<promise_type>;
        handle coro_handle_;

        explicit base(std::coroutine_handle<promise_type> h) : coro_handle_(h) {}

        struct promise_type {
            base get_return_object() {
                return base{handle::from_promise(*this)};
            }

            std::suspend_always initial_suspend() { return {}; }

            std::suspend_always final_suspend() noexcept {

                fmt::print("base final suspend\n");
                return {};
            }

            void return_void() {
                fmt::print("returning\n");
            }

            void unhandled_exception() {}
        };
    };
}
#endif //SOCKETS_BASE_TASK_HPP
