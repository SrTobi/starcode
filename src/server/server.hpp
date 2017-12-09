#pragma once

#include "defs.hpp"

class Server {
public:
    virtual ~Server() = default;

    virtual void start(int port) = 0;

    static std::unique_ptr<Server> create();
};