// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#ifndef __GRPC_ROUTE_CLASSIFY_IMPL_H
#define __GRPC_ROUTE_CLASSIFY_IMPL_H

#include <grpcpp/grpcpp.h>
#include <grpc/grpc.h>
#include "grpc_nmt.grpc.pb.h"
#include "grpc_nmt.pb.h"

#include "nmt.h"
#include "utils.h"

using grpc::Status;

class GrpcRouteNMTImpl : public RouteNMT::Service {
public:
    GrpcRouteNMTImpl(shared_ptr<nmt> nmt_ptr, int debug_mode);
    ~GrpcRouteNMTImpl() {};
private:
    grpc::Status GetDomains(grpc::ServerContext* context,
                            const Empty* request,
                            Domains* response) override;
    grpc::Status GetNMT(grpc::ServerContext* context,
                            const TextToParse* request,
                            TextParsed* response) override;
    grpc::Status StreamNMT(grpc::ServerContext* context,
                                grpc::ServerReaderWriter< TextParsed, TextToParse>* stream) override;

    void PrepareOutput(const TextToParse* request, TextParsed* response);

    void SetOutput(TextParsed* response, std::vector<std::vector<std::string>>& tokenized_batch, std::vector<std::vector<std::string>>& output_batch_tokens);

    shared_ptr<nmt> _nmt;
    int _debug_mode;
};

#endif // __GRPC_ROUTE_CLASSIFY_IMPL_H
