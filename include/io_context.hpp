
#ifndef SOCKETS_IO_CONTEXT_HPP
#define SOCKETS_IO_CONTEXT_HPP

#include <io_uring.hpp>
#include <task.hpp>
#include <base_task.hpp>
#include <accept_data.hpp>

#include <vector>
#include <coroutine>

namespace harmony {
    class io_context {
    public:
        explicit io_context(int queue_depth = 80):
            active_tasks_{std::make_shared<int>(0)},
            ring_{iouring(queue_depth)}{
        }
        template<typename T>
        void post_task(task<T> new_task) {
            auto top_level_task = start_task(std::move(new_task));
            base_tasks_.emplace_back(top_level_task.coro_handle_);
            run_queue_.emplace_back(queue_item{top_level_task.coro_handle_});
        }
        void run() {
            for (;;) {
                while (!run_queue_.empty()) {
                    auto h = run_queue_.front();
                    h.handle_.resume();
                    run_queue_.pop_front();
                }

                if (*active_tasks_ == 0) {
                    break;
                }

                ring_.submit_and_wait(run_queue_);
            }
        }
        harmony::iouring &ring() { return ring_; }
        ~io_context(){
            for (const auto& base_task: base_tasks_){
                base_task.destroy();
            }
        }
    private:
        template<typename T> base start_task(task<T> task) {
            task.increment(active_tasks_);
            co_return co_await task;
        };
        std::deque<queue_item> run_queue_;
        std::vector<std::coroutine_handle<>> base_tasks_;
        std::shared_ptr<int> active_tasks_;
        harmony::iouring ring_;
    };
}
#endif // SOCKETS_IO_CONTEXT_HPP