// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#ifndef __GRPC_SERVER_H
#define __GRPC_SERVER_H

#include <algorithm>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <sstream>
#include <time.h>

#include "abstract_server.h"
#include "grpc_route_nmt_impl.h"

using namespace std;
using namespace nlohmann;

class grpc_server : public AbstractServer {

public:
    using AbstractServer::AbstractServer;
    ~grpc_server() {delete _service;}
    void init(size_t thr = 2) override;
    void start() override;
    void shutdown() override;

private:
    GrpcRouteNMTImpl *_service;
};

#endif // __GRPC_SERVER_H
