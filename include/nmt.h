// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#pragma once
#include <iostream>
#include <boost/locale.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <fstream>
#include <fcntl.h>

#include "grpcpp/create_channel.h"

#include <grpcpp/security/credentials.h>
#include <google/protobuf/map.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "tensorflow_serving/apis/prediction_service.grpc.pb.h"
#include "tensorflow_serving/apis/model_service.grpc.pb.h"

#include "tokenizer.h"
#include "utils.h"
#include "spm.h"
#include "easywsclient/easywsclient.h"


#include <grpcpp/grpcpp.h>



using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using tensorflow::serving::PredictRequest;
using tensorflow::serving::PredictResponse;
using tensorflow::serving::PredictionService;
using tensorflow::serving::ModelService;

typedef google::protobuf::Map<std::string, tensorflow::TensorProto> OutMap;


using namespace std;
using easywsclient::WebSocket;


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


class nmt
{
    public:
        nmt();
        ~nmt(){delete(_spm_src);delete(_spm_tgt);};
        nmt(std::string& model_name_param, std::string& address_server, std::string& spm_file_model, std::string& lang_src, std::string& lang_tgt, bool tensorflow_serving_type);
        bool LoadModel(std::string model_name_param, std::string& address_server, std::string& spm_file_model, std::string& lang_src, std::string& lang_tgt, bool tensorflow_serving_type);
        bool NMTTranslate(std::string& sentence_to_translate, std::vector< std::string >& translation_output, std::vector< float >& output_translation_scores, std::vector< std::string >& output_alignement_scores);
        bool NMTTranslateBatch(std::vector< std::string >& batch_sentence_to_translate, std::vector< std::string >& translation_output, std::vector< float >& output_translation_scores, std::vector< std::string >& output_alignement_scores);
        bool getLocal();
        Status NMTBatch(std::vector<std::vector<std::string> >& batch_tokens, std::vector<std::vector<std::string> >& output_batch_tokens, std::string domain);        
        bool CheckModelsStatus();
        std::string getDomain(){return _model_name;};
        std::vector<std::string> getDomains();
        std::string getLangSrc(){return _lang_src;};
        std::string getLangTgt(){return _lang_tgt;};
        std::vector <std::string> tokenize(std::string &input);
        std::string tokenize_str(std::string &input);
        std::string spm_segment(std::string &input);
        std::string detokenize_str(std::string& input);
        std::string spm_detokenize_str(std::string& input);
        void setDebugMode(int debug_mode);
        
    private:
      bool _local;
      bool _is_model_tfserving;
      int _debug_mode;
      spm * _spm_src;
      spm * _spm_tgt;
      string _model_name;
      string _address;
      string _lang_src;
      string _lang_tgt;
      
      static std::map<tensorflow::serving::ModelVersionStatus_State, std::string> mapState;

      unique_ptr<PredictionService::Stub> _stub;
      shared_ptr<Channel> _channel;

      std::string _model_config_path;

      std::vector<int> PadBatch(std::vector<std::vector<std::string> >& batch_tokens);
      int getMaxLengthWord(std::vector<std::vector<std::string>>& batch_tokens);
      void getBatchCharsListFromBatchTokens(std::vector<std::vector<std::vector<std::string>>>& chars_list_batch, std::vector<std::vector<std::string>>& batch_tokens, int max_length_word);
    
  
};
