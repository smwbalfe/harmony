//
// Created by shriller44 on 1/8/24.
//

#ifndef SOCKETS_TASK_HPP
#define SOCKETS_TASK_HPP
#include <fmt/format.h>
#include <atomic>
#include <exception>
#include <utility>
#include <type_traits>
#include <cstdint>
#include <cassert>
#include <coroutine>

namespace harmony {

    template<typename T>
    class task;

    namespace detail {
        class task_promise_base {

            friend struct final_awaitable;

            struct final_awaitable {
                bool await_ready() const noexcept {
                    return false;
                }

                template<typename PROMISE>
                std::coroutine_handle<> await_suspend(
                        std::coroutine_handle<PROMISE> coro) noexcept {
                    if (coro.promise().active_tasks_) {
                        (*coro.promise().active_tasks_)--;
                    }
                    return coro.promise().m_continuation;
                }

                void await_resume() noexcept {
                    fmt::print("resuming\n");
                }
            };

        public:

            std::shared_ptr<int> active_tasks_ = nullptr;

            task_promise_base() noexcept {}

            auto initial_suspend() noexcept {

                return std::suspend_always{};
            }

            auto final_suspend() noexcept {
                return final_awaitable{};
            }

            void set_continuation(std::coroutine_handle<> continuation) noexcept {
                m_continuation = continuation;
            }

        private:

            std::coroutine_handle<> m_continuation;

        };

        template<typename T>
        class task_promise final : public task_promise_base {
        public:

            task_promise() noexcept {}

            ~task_promise() {
                switch (m_resultType) {
                    case result_type::value:
                        m_value.~T();
                        break;
                    case result_type::exception:
                        m_exception.~exception_ptr();
                        break;
                    default:
                        break;
                }
            }

            task<T> get_return_object() noexcept;

            void unhandled_exception() noexcept {
                ::new(static_cast<void *>(std::addressof(m_exception))) std::exception_ptr(
                        std::current_exception());
                m_resultType = result_type::exception;
            }


            template<typename VALUE, typename = std::enable_if_t<std::is_convertible_v<VALUE &&, T>>>
            void return_value(VALUE &&value) noexcept(std::is_nothrow_constructible_v<T, VALUE &&>) {
                //m_value =  T(std::forward<VALUE>(value));
                ::new(static_cast<void *>(std::addressof(m_value))) T(std::forward<VALUE>(value));
                m_resultType = result_type::value;
            }

            T &result() &{
                if (m_resultType == result_type::exception) {
                    std::rethrow_exception(m_exception);
                }

                assert(m_resultType == result_type::value);

                return m_value;
            }


            using rvalue_type = std::conditional_t<
            std::is_arithmetic_v<T> || std::is_pointer_v<T>,
            T,
            T &&>;

            rvalue_type result() &&{
                if (m_resultType == result_type::exception) {
                    std::rethrow_exception(m_exception);
                }

                assert(m_resultType == result_type::value);

                return std::move(m_value);
            }

        private:

            enum class result_type {
                empty, value, exception
            };

            result_type m_resultType = result_type::empty;

            union {
                T m_value;
                std::exception_ptr m_exception;
            };

        };

        template<>
        class task_promise<void> : public task_promise_base {
        public:

            task_promise() noexcept = default;

            task<void> get_return_object() noexcept;

            void return_void() noexcept {}

            void unhandled_exception() noexcept {
                m_exception = std::current_exception();
            }

            void result() {
                if (m_exception) {
                    std::rethrow_exception(m_exception);
                }
            }

        private:

            std::exception_ptr m_exception;

        };

        template<typename T>
        class task_promise<T &> : public task_promise_base {
        public:

            task_promise() noexcept = default;

            task<T &> get_return_object() noexcept;

            void unhandled_exception() noexcept {
                m_exception = std::current_exception();
            }

            void return_value(T &value) noexcept {
                m_value = std::addressof(value);
            }

            T &result() {
                if (m_exception) {
                    std::rethrow_exception(m_exception);
                }

                return *m_value;
            }

        private:

            T *m_value = nullptr;
            std::exception_ptr m_exception;

        };
    }

    template<typename T = void>
    class [[nodiscard]] task {
    public:

        using promise_type = detail::task_promise<T>;

        using value_type = T;

    private:

        struct awaitable_base {
            std::coroutine_handle<promise_type> m_coroutine;

            awaitable_base(std::coroutine_handle<promise_type> coroutine) noexcept
                    : m_coroutine(coroutine) {}

            bool await_ready() const noexcept {
                return !m_coroutine || m_coroutine.done();
            }

            template<typename Promise>
            std::coroutine_handle<> await_suspend(
                    std::coroutine_handle<Promise> awaitingCoroutine) noexcept {

                // fmt::print("awaitable_base await_suspend\n");

                //  in our example this is the handle of the base task.
                m_coroutine.promise().set_continuation(awaitingCoroutine);

                // we are suspended at initial suspend, this resumes execution like a tail-resume
                // we return the handle from await_suspend.
                return m_coroutine;
            }
        };

    public:

        task() noexcept
                : m_coroutine(nullptr) {}

        explicit task(std::coroutine_handle<promise_type> coroutine)
                : m_coroutine(coroutine) {}

        task(task &&t) noexcept
                : m_coroutine(t.m_coroutine) {
            t.m_coroutine = nullptr;
        }

        task(const task &) = delete;

        task &operator=(const task &) = delete;

        ~task() {
            if (m_coroutine) {
                m_coroutine.destroy();
            }
        }

        void increment(std::shared_ptr<int> active_tasks_){
            (*active_tasks_)++;
            m_coroutine.promise().active_tasks_ = std::move(active_tasks_);
        }

        task &operator=(task &&other) noexcept {
            if (std::addressof(other) != this) {
                if (m_coroutine) {
                    m_coroutine.destroy();
                }

                m_coroutine = other.m_coroutine;
                other.m_coroutine = nullptr;
            }

            return *this;
        }

        bool is_ready() const noexcept {
            return !m_coroutine || m_coroutine.done();
        }

        auto operator
        co_await() const & noexcept
        {

            struct awaitable : awaitable_base {
                using awaitable_base::awaitable_base;

                decltype(auto) await_resume() {
                    if (!this->m_coroutine) {
                        fmt::print("throw");
                    }

                    return this->m_coroutine.promise().result();
                }
            };

            return awaitable{m_coroutine};
        }

        auto operator
        co_await() const && noexcept
        {
            struct awaitable : awaitable_base {
                using awaitable_base::awaitable_base;

                decltype(auto) await_resume() {
                    if (!this->m_coroutine) {
                        fmt::print("throwing broken promise");
                    }

                    return std::move(this->m_coroutine.promise()).result();
                }
            };

            return awaitable{m_coroutine};
        }

        /// \brief
        /// Returns an awaitable that will await completion of the task without
        /// attempting to retrieve the result.
        auto when_ready() const noexcept {
            struct awaitable : awaitable_base {
                using awaitable_base::awaitable_base;

                void await_resume() const noexcept {}
            };

            return awaitable{m_coroutine};
        }

    private:
        std::coroutine_handle<promise_type> m_coroutine;

    };

    // get_return_object returns a task type dependent on whether there is
    // void , reference or copy
    namespace detail {
        template<typename T>
        task<T> task_promise<T>::get_return_object() noexcept {
            return task<T>{std::coroutine_handle<task_promise>::from_promise(*this)};
        }

        inline task<void> task_promise<void>::get_return_object() noexcept {

            return task<void>{std::coroutine_handle<task_promise>::from_promise(*this)};
        }
        template<typename T>
        task<T &> task_promise<T &>::get_return_object() noexcept {
            return task<T &>{std::coroutine_handle<task_promise>::from_promise(*this)};
        }
    }

//    template<typename AWAITABLE>
//    auto make_task(AWAITABLE awaitable)
//    -> task<detail::remove_rvalue_reference_t<typename awaitable_traits<AWAITABLE>::await_result_t>>
//    {
//        co_return co_await static_cast<AWAITABLE&&>(awaitable);
//    }

}
#endif //SOCKETS_TASK_HPP
