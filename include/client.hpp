//
// Created by shriller44 on 5/4/24.
//

#ifndef HARMONY_CLIENT_HPP
#define HARMONY_CLIENT_HPP

#include <endpoint.hpp>

namespace harmony {
    struct client {
        int fd_;
        std::unique_ptr <endpoint> ep_;
    };
}
#endif //HARMONY_CLIENT_HPP
