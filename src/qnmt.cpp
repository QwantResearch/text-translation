/***************************************************************************
* MIT License
* 
* Copyright (c) 2018 Christophe SERVAN, Qwant Research, 
* email: christophe[dot]servan[at]qwantresearch[dot]com
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
***************************************************************************/
#include "qnmt.h"

using namespace std;


void Split_str(const std::string& line, std::vector<std::string>& pieces, const std::string del) {
  size_t begin = 0;
  size_t pos = 0;
  std::string token;
  while ((pos = line.find(del, begin)) != std::string::npos) {
    if (pos > begin) {
      token = line.substr(begin, pos - begin);
      if(token.size() > 0)
        pieces.push_back(token);
    }
    begin = pos + del.size();
  }
  if (pos > begin) {
    token = line.substr(begin, pos - begin);
  }
  if(token.size() > 0)
    pieces.push_back(token);
}

qnmt::qnmt(const tensorflow::string& export_dir)
{
    LoadModel(export_dir);
    _local=true;
}
qnmt::qnmt()
{
    cerr << "Warning: NMT object is empty" <<endl;
    _local=true;
}

bool qnmt::getLocal()
{
    return _local;
}


bool qnmt::LoadModel(std::string model_name_param,shared_ptr<grpc::Channel> channel)
{
    _model_name=model_name_param;
    _stub=PredictionService::NewStub(channel);
    _local=false;
}


// Displays a batch of tokens.
void qnmt::PrintBatch(
    const std::vector<std::vector<tensorflow::string> >& batch_tokens) {
  for (const auto& tokens : batch_tokens) {
    for (const auto& token : tokens) {
      std::cout << " " << token;
    }
    std::cout << std::endl;
  }
}

// Pads a batch of tokens and returns the length of each sequence.
std::vector<tensorflow::int32> qnmt::PadBatch(
    std::vector<std::vector<tensorflow::string> >& batch_tokens) {
  std::vector<tensorflow::int32> lengths;
  size_t max_length = 0;

  lengths.reserve(batch_tokens.size());

  for (const auto& tokens : batch_tokens) {
    lengths.push_back(static_cast<tensorflow::int32>(tokens.size()));
    max_length = std::max(max_length, tokens.size());
  }
  for (auto& tokens : batch_tokens) {
    tokens.insert(tokens.end(), max_length - tokens.size(), "");
  }

  return lengths;
}


// Loads a saved model.
bool qnmt::LoadModel(const tensorflow::string& export_dir) {
  tensorflow::SessionOptions session_options;
  tensorflow::RunOptions run_options;

  tensorflow::Status load_saved_model_status =
      tensorflow::LoadSavedModel(session_options, run_options, export_dir,
                     {tensorflow::kSavedModelTagServe}, &_bundle);

  if (!load_saved_model_status.ok()) {
    std::cerr << load_saved_model_status << std::endl;
    return false;
  }

  return true;
}

// Tanslates a batch of tokenizes sentences.
bool qnmt::NMTBatch(
    std::vector<std::vector<tensorflow::string> > batch_tokens,
    std::vector<std::vector<tensorflow::string> >& output_batch_tokens) {
  // Pad batch.
  std::vector<tensorflow::int32> lengths = PadBatch(batch_tokens);

  tensorflow::int64 batch_size = batch_tokens.size();
  tensorflow::int64 max_length = batch_tokens.front().size();
  
  std::vector<tensorflow::string> flat_batch_tokens = FlattenVector(batch_tokens);
  tensorflow::Tensor tokens_tensor = AsTensor(flat_batch_tokens, {batch_size, max_length});
  tensorflow::Tensor lengths_tensor = AsTensor(lengths);

  // Resolve name of inputs to fed and outputs to fetch.
  const auto signature_def_map = _bundle.meta_graph_def.signature_def();
  const auto signature_def = signature_def_map.at(tensorflow::kDefaultServingSignatureDefKey);
  const tensorflow::string tokens_input_name = signature_def.inputs().at("tokens").name();
  const tensorflow::string length_input_name = signature_def.inputs().at("length").name();
  const tensorflow::string tokens_output_name = signature_def.outputs().at("tokens").name();
  const tensorflow::string length_output_name = signature_def.outputs().at("length").name();

  // Forward in the graph.
  std::vector<tensorflow::Tensor> outputs;
  tensorflow::Status run_status = _bundle.session->Run({{tokens_input_name, tokens_tensor}, {length_input_name, lengths_tensor}}, {tokens_output_name, length_output_name}, {}, &outputs);

  if (!run_status.ok()) {
    std::cerr << "Running model failed: " << run_status << std::endl;
    return false;
  }

  // Convert TensorFlow tensors to Eigen tensors.
  auto e_tokens = outputs[0].tensor<tensorflow::string,3>();
  auto e_length = outputs[1].matrix<tensorflow::int32>();
  // Collect results in C++ vectors.
  for (long b = 0; b < batch_size; ++b) 
  {
      long len = e_length(b, 0);
      std::vector<tensorflow::string> output_tokens;
      output_tokens.reserve(len);
      for (long i = 0; i < len - 1; ++i) 
      {
          output_tokens.push_back(e_tokens(b, 0, i));
      }
      output_batch_tokens.push_back(output_tokens);
  }

  return true;
}



// Tanslates a batch of tokenizes sentences.
bool qnmt::NMTBatchOnline(
    std::vector<std::vector<tensorflow::string> > batch_tokens,
    std::vector<std::vector<tensorflow::string> >& output_batch_tokens) {
  // Pad batch.
  // Pad batch.
  std::vector<tensorflow::int32> lengths = PadBatch(batch_tokens);

  tensorflow::int64 batch_size = batch_tokens.size();
  tensorflow::int64 max_length = batch_tokens.front().size();
  
  std::vector<tensorflow::string> flat_batch_tokens = FlattenVector(batch_tokens);
  tensorflow::Tensor tokens_tensor = AsTensor(flat_batch_tokens, {batch_size, max_length});
  tensorflow::Tensor lengths_tensor = AsTensor(lengths);


  tensorflow::TensorProto* tokens_tensor_proto = new tensorflow::TensorProto();
  tensorflow::TensorProto* lengths_tensor_proto = new tensorflow::TensorProto();

  PredictRequest predictRequest;
  PredictResponse response;
  ClientContext context;

  predictRequest.mutable_model_spec()->set_name(_model_name);

//   google::protobuf::Map< std::string, tensorflow::Tensor >& inputs = *predictRequest.mutable_inputs();
  google::protobuf::Map< std::string, tensorflow::TensorProto >& inputs = *(predictRequest.mutable_inputs());
  tokens_tensor.AsProtoField(tokens_tensor_proto);
  lengths_tensor.AsProtoField(lengths_tensor_proto);
  
//   =tensorflow::ops::SerializeTensor(l_scope,tokens_tensor);
//   inputs["tokens"]=::Output();
//   inputs["chars"]=tensorflow::ops::SerializeTensor(l_scope ,chars_tensor);
//   inputs["length"]=tensorflow::ops::SerializeTensor(l_scope ,lengths_tensor);
  inputs["tokens"]=(*tokens_tensor_proto);
  inputs["length"]=(*lengths_tensor_proto);

//       cerr << "Asking Stub though predict !!!" <<endl;

  Status status = _stub->Predict(&context, predictRequest, &response);
  
//   std::cerr << "check status.." << std::endl;
  
  if (status.ok()) 
  {
//     std::cout << "call predict ok" << std::endl;
//     std::cout << "outputs size is "<< response.outputs_size() << std::endl;
    OutMap& map_outputs =  *response.mutable_outputs();
    OutMap::iterator iter;
    int output_index = 0;
//     tensorflow::Tensor tensor_tmp;
    tensorflow::Tensor e_tokens;
    tensorflow::Tensor e_length;
    
    for(iter = map_outputs.begin();iter != map_outputs.end(); ++iter)
    {
      tensorflow::TensorProto& result_tensor_proto= iter->second;
      tensorflow::Tensor tensor;

      bool converted = tensor.FromProto(result_tensor_proto);
      if (converted) {
        if (iter->first.find("tokens") == 0) e_tokens.FromProto(result_tensor_proto);
        if (iter->first.find("length") == 0) e_length.FromProto(result_tensor_proto);
//         std::cout << "the " <<iter->first <<" result tensor[" << output_index << "] is:" <<
//               std::endl << tensor.SummarizeValue(250) << std::endl;
      }else {
        std::cerr << "the " <<iter->first <<" result tensor[" << output_index << 
              "] convert failed." << std::endl;
      }
      ++output_index;
    }
// 
  // Collect results in C++ vectors.
  
  // Collect results in C++ vectors.
  long prev=0;
  vector<string> vec_length;
  vector<string> vec_tokens;
  Split_str(e_length.SummarizeValue(250),vec_length, " ");
  Split_str(e_tokens.SummarizeValue(250),vec_tokens, " ");
  for (long b = 0; b < batch_size; ++b) {
    long len = prev+atol(vec_length[b].c_str());
//     prev=
    std::vector<tensorflow::string> output_tokens;
    output_tokens.reserve(len);
    for (long i = prev; i < len ; ++i) {
      output_tokens.push_back(vec_tokens[i]);
    }
    prev=len;
    output_batch_tokens.push_back(output_tokens);
  }  


  return "Done.";
  } else {
    std::cout << "gRPC call return code: " 
        <<status.error_code() << ": " << status.error_message()
        << std::endl;
    return "gRPC failed.";
  }
 
  return true;
}

/*
int main(int argc, char* argv[]) {
  tensorflow::string export_dir;

  std::vector<tensorflow::Flag> flag_list = {tensorflow::Flag(
      "export_dir", &export_dir, "directory containing the exported model")};

  tensorflow::string usage = tensorflow::Flags::Usage(argv[0], flag_list);
  const bool parse_result = tensorflow::Flags::Parse(&argc, argv, flag_list);
  if (export_dir.size() == 0) {
    std::cerr << usage << std::endl;
    return 1;
  }
  cerr << "Export Dir : "  << export_dir << endl;
  tensorflow::port::InitMain(argv[0], &argc, &argv);

  tensorflow::SavedModelBundle bundle;
  std::vector<std::vector<tensorflow::string> > output_batch_tokens;
  std::vector<std::vector<tensorflow::string> > batch_tokens = {
      {"bonjour", "je", "voudrais", "réserver", "un", "hôtel", "à", "Paris"},
      {"Quel", "est", "le", "taux", "de", "pollution", "à", "Lyon", "?"}};

  if (!LoadModel(bundle, export_dir)) {
    return 1;
  }

  std::cout << "Input:" << std::endl;
  PrintBatch(batch_tokens);

  if (!NMTBatch(batch_tokens, output_batch_tokens)) {
    return 1;
  }

  std::cout << "Output:" << std::endl;
  PrintBatch(output_batch_tokens);

  return 0;
}*/