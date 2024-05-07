# Harmony

- Harmony is an asynchronous web framework based on C++20 coroutines and Linux 5.1 io_uring (work in progress).
- The goal was to create a simple HTTP web framework based on top of this asynchronous runtime.

```cpp
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
```

### Build

- Must be on a Linux machine running a kernel with at least version 5.1.
- Install vcpkg.
- Enter `vcpkg install` 
- Pass in the `vcpkg.cmake` as a `CMAKE_TOOLCHAIN_FILE` variable to correctly utilise all the installed dependencies.

### Details

1. The async runtime (coroutines + io_uring) 

   - Coroutines are functions that utilise coroutine syntax operations (`co_yield`, `co_await` and `co_return`). They are like regular functions, but we have the ability to augment the control flow by suspending and resuming its execution at various points, referred to as *suspend points*.
   - In this application, I utilised `harmony::task<T>` type to encapsulate the machinery required for the coroutines to operate. This is based off of Lewis bakers work ==reference== 
   - Coroutines can `co_await` other coroutines, this feature is enabled by the usage of the `harmony::task<T>` types symmetric transfer where on coroutine being suspended transfers control flow to another coroutine by resuming its handle.
   - For the server there are 3 core *awaitables* required. An awaitable is any class that has support for `co_await` being called on it. Each of the following awaitables utilise **io_uring** to handle kernel level socket operations in an asynchronous manner.

     - `async_accept` - Accept new clients.
     - `async_send ` - Send bytes to a socket.
     - `async_recv`  - Receive bytes from a socket.
   - The key with each awaitable is that it attaches the handle of the coroutine to its context. This means that when we receive completion events from the io_uring we can resume the handle.
   - To utilise the power of async operations, we will have multiple coroutines being managed on one thread of execution, so its necessary to have a *coroutine scheduler*. This is essentially a queue of coroutine handles `std::coroutine_handle`.
   - Scheduler flow
     - We initialise the scheduler by *posting tasks* i.e booting off an initial coroutine for it to run.
     - Our scheduler will run continuously as long as there is at least one active task (coroutine). 
     - At each iteration we read from the front of the queue and resume the coroutine to let it run to the next suspend point, from which it hands back control to the scheduler control flow. 
     - We then remove this coroutine from the queue.
     - We now wait on **io_uring** events, this will be an array of events submitted when we `co_await` 
2. HTTP server
   - The user can register routes and the desired resource to send from this.
   - The HTTP server is organised by a variety of coroutines that `co_await` each step of a request
     - accept client
     - receives bytes for the header / request to parse
     - send bytes for the header / request response.
   - The parser stage will simply extract the key value pairs and body into a `harmony::request` object that is fed into each route handler.
   - The builder stage will construct the required headers and resource body. The response is serialised to a string using a method that ensures we are as efficient as possible in having the correct allocated size for the string each time to avoid redundant memory reallocation.

### Future plans

- JSON ==high priority== 

- HTTPS
- Thread Pool for parsing and building HTTP requests / responses

