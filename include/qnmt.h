// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#pragma once
#include <iostream>
#include <boost/locale.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <tensorflow/cc/saved_model/loader.h>
#include <tensorflow/cc/saved_model/signature_constants.h>
#include <tensorflow/cc/saved_model/tag_constants.h>

#include <tensorflow/cc/framework/scope.h>

#include <tensorflow/cc/ops/parsing_ops.h>
#include <tensorflow/core/lib/io/path.h>
#include <tensorflow/core/platform/init_main.h>
#include <tensorflow/core/util/command_line_flags.h>


#include "grpcpp/create_channel.h"
#include "grpcpp/security/credentials.h"
#include "google/protobuf/map.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/util/command_line_flags.h"
#include "tensorflow_serving/apis/prediction_service.grpc.pb.h"

#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "utils.h"

// #include "qsmt.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using tensorflow::serving::PredictRequest;
using tensorflow::serving::PredictResponse;
using tensorflow::serving::PredictionService;

// using tensorflow::Flag;
// using tensorflow::Tensor;
// using tensorflow::Status;
// using tensorflow::string;
// using tensorflow::int32;

typedef google::protobuf::Map<tensorflow::string, tensorflow::TensorProto> OutMap;


using namespace std;

// Flattens a 2D std::vector to a 1D std::vector.
template <typename T>
std::vector<T> FlattenVector(const std::vector<std::vector<T> >& vals) {
  std::vector<T> flat_vals;
  flat_vals.reserve(vals.size() * vals.front().size());
  for (const auto& v : vals) {
    flat_vals.insert(flat_vals.end(), v.cbegin(), v.cend());
  }
  return flat_vals;
}
// Flattens a 3D std::vector to a 1D std::vector.
template <typename T>
std::vector<T> FlattenVector(const std::vector<std::vector<std::vector<T> > >& vals) {
  std::vector<T> flat_vals;
  size_t final_size = vals.size() * vals.front().size() * vals.front().front().size();
//   flat_vals.reserve(vals.size() * vals.front().size() * vals.front().front().size() );
  for (const auto& values : vals) 
  {
      for (const auto& v : values) 
      {
          flat_vals.insert(flat_vals.end(), v.cbegin(), v.cend());
      }
  }
  return flat_vals;
}

// Convenience functions to convert std::vectors to tensorflow::Tensors. 
template <typename T>
tensorflow::Tensor AsTensor(const std::vector<T>& vals) {
  tensorflow::Tensor ret(tensorflow::DataTypeToEnum<T>::value,
                        {static_cast<tensorflow::int64>(vals.size())});
  std::copy_n(vals.data(), vals.size(), ret.flat<T>().data());
  return ret;
}

template <typename T>
tensorflow::Tensor AsTensor(const std::vector<T>& vals,
                            const tensorflow::TensorShape& shape) {
  tensorflow::Tensor ret;
  ret.CopyFrom(AsTensor(vals), shape);
  return ret;
}

class qnmt
{
    public:
        qnmt();
        qnmt(const tensorflow::string&  filename);
        void PrintBatch(const std::vector<std::vector<tensorflow::string> >& batch_tokens);
        std::vector<tensorflow::int32> PadBatch(std::vector<std::vector<tensorflow::string> >& batch_tokens);
        bool LoadModel(const tensorflow::string& export_dir);
        bool LoadGraph(const std::string& graph_file);
        tensorflow::Status LoadGraph(const string& graph_file_name, std::unique_ptr<tensorflow::Session>* session);
        bool LoadModel(std::string model_name_param,shared_ptr<grpc::Channel> channel);
//         bool LoadModel(std::string& address, std::string& iport, std::string& route, std::string& name);
        bool NMTBatch(std::vector<std::vector<tensorflow::string> > batch_tokens, std::vector<std::vector<tensorflow::string> >& output_batch_tokens, std::vector<float>& output_batch_scores);
//         bool NMTBatchSMT(std::vector<std::vector<tensorflow::string> > batch_tokens, std::vector<std::vector<tensorflow::string> >& output_batch_tokens, std::vector<float>& output_batch_scores);
        bool NMTBatchGraph(std::vector<std::vector<tensorflow::string> > batch_tokens, std::vector<std::vector<tensorflow::string> >& output_batch_tokens, std::vector<float>& output_batch_scores);
        bool NMTBatchOnline(std::vector<std::vector<tensorflow::string> > batch_tokens,std::vector<std::vector<tensorflow::string> >& output_batch_tokens, std::vector<float>& output_batch_scores);
        bool getLocal();
    private:
      tensorflow::SavedModelBundle _bundle;
      std::unique_ptr<tensorflow::Session> _session;
      shared_ptr<PredictionService::Stub> _stub;
//       qsmt* _qsmt;
      bool _local;
      string _model_name;
  
};
