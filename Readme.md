### Harmony Project Architecture

#### Overview
- Harmony is a C++ project aimed at delivering a high-performance, asynchronous networking and HTTP server framework.
- It emphasizes the use of modern C++ coroutines to facilitate asynchronous programming, alongside Linux's IO_uring for efficient IO operations.

#### Core Components

- **Asynchronous Runtime with Coroutines**
  - `base_task.hpp`: Defines the base interface for tasks, leveraging C++ coroutines for asynchronous operations.
  - `io_context.hpp`: Manages the IO context, orchestrating the scheduling and execution of coroutine-based tasks.
  - `io_uring.hpp`: Integrates Linux's IO_uring interface to provide efficient, non-blocking IO operations suitable for coroutine suspension and resumption.
  - `request_data.hpp`: Encapsulates request-related data, utilized across the asynchronous runtime to manage task state and data flow.
  - `task.hpp`: Implements specific asynchronous tasks using coroutines, extending the `base_task` interface for various operations.

- **Networking with Coroutines**
  - `endpoint.hpp`: Abstract base for network endpoints, designed to work seamlessly with coroutine-based asynchronous operations.
  - `ipv4.hpp`: Provides IPv4 endpoint functionality, optimized for asynchronous operations with coroutines.
  - `ipv6.hpp`: Provides IPv6 endpoint functionality, similarly optimized for coroutine-based networking.
  - `async_client_acceptor.hpp`: Handles the asynchronous acceptance of new client connections using coroutines for non-blocking operations.
  - `async_socket.hpp`: Supports non-blocking socket operations, fully integrated with C++ coroutines for efficient networking.

- **HTTP Processing**
  - `http_builder.hpp`: Constructs HTTP responses, designed to work within coroutine-based asynchronous flows.
  - `http_parser.hpp`: Parses incoming HTTP requests, optimized for integration with coroutine-based tasks.

- **Server**
  - `server.hpp`: The core server implementation that brings together asynchronous runtime, networking, and HTTP processing components, all utilizing coroutines for asynchronous logic.
  - `server.cpp`: The server's entry point, setting up and running the coroutine-based asynchronous service.

#### Build System
- `CMakeLists.txt`: Configures the build system, specifying project requirements, and linking dependencies, ensuring support for C++ coroutines.

#### Getting Started
- Detailed instructions on building and running the Harmony server, highlighting the use of C++ coroutines and prerequisites for leveraging IO_uring.


##### Technologies used:

- C++
- C++20 Coroutines
- Linux 5.1 io_uring
- Linux Berkeley sockets.

