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

std::string Join_str(const std::vector<std::string>& words, const std::string del) 
{
  std::stringstream ss;
  if (words.empty()) {
    return "";
  }
  ss << words[0];
  for (size_t i = 1; i < words.size(); ++i) {
    ss << del << words[i];
  }
  return ss.str();
}

void str_to_utf8(string& line)
{
    boost::replace_all(line, "\\'", "'");
    int line_size=(int)line.size();
    for (int i=0; i< line_size; i++)
//     while ((int)line.find("\\") > -1)
    {
        boost::replace_all(line, "\\302\\241", "¡");
        boost::replace_all(line, "\\302\\242", "¢");
        boost::replace_all(line, "\\302\\243", "£");
        boost::replace_all(line, "\\302\\244", "¤");
        boost::replace_all(line, "\\302\\245", "¥");
        boost::replace_all(line, "\\302\\246", "¦");
        boost::replace_all(line, "\\302\\247", "§");
        boost::replace_all(line, "\\302\\250", "¨");
        boost::replace_all(line, "\\302\\251", "©");
        boost::replace_all(line, "\\302\\252", "ª");
        boost::replace_all(line, "\\302\\253", "«");
        boost::replace_all(line, "\\302\\254", "¬");
        boost::replace_all(line, "\\302\\255", "­");
        boost::replace_all(line, "\\302\\256", "®");
        boost::replace_all(line, "\\302\\257", "¯");
        boost::replace_all(line, "\\302\\260", "°");
        boost::replace_all(line, "\\302\\261", "±");
        boost::replace_all(line, "\\302\\262", "²");
        boost::replace_all(line, "\\302\\263", "³");
        boost::replace_all(line, "\\302\\264", "´");
        boost::replace_all(line, "\\302\\265", "µ");
        boost::replace_all(line, "\\302\\266", "¶");
        boost::replace_all(line, "\\302\\267", "·");
        boost::replace_all(line, "\\302\\270", "¸");
        boost::replace_all(line, "\\302\\271", "¹");
        boost::replace_all(line, "\\302\\272", "º");
        boost::replace_all(line, "\\302\\273", "»");
        boost::replace_all(line, "\\302\\274", "¼");
        boost::replace_all(line, "\\302\\275", "½");
        boost::replace_all(line, "\\302\\276", "¾");
        boost::replace_all(line, "\\302\\277", "¿");
        boost::replace_all(line, "\\303\\200", "À");
        boost::replace_all(line, "\\303\\201", "Á");
        boost::replace_all(line, "\\303\\202", "Â");
        boost::replace_all(line, "\\303\\203", "Ã");
        boost::replace_all(line, "\\303\\204", "Ä");
        boost::replace_all(line, "\\303\\205", "Å");
        boost::replace_all(line, "\\303\\206", "Æ");
        boost::replace_all(line, "\\303\\207", "Ç");
        boost::replace_all(line, "\\303\\210", "È");
        boost::replace_all(line, "\\303\\211", "É");
        boost::replace_all(line, "\\303\\212", "Ê");
        boost::replace_all(line, "\\303\\213", "Ë");
        boost::replace_all(line, "\\303\\214", "Ì");
        boost::replace_all(line, "\\303\\215", "Í");
        boost::replace_all(line, "\\303\\216", "Î");
        boost::replace_all(line, "\\303\\217", "Ï");
        boost::replace_all(line, "\\303\\220", "Ð");
        boost::replace_all(line, "\\303\\221", "Ñ");
        boost::replace_all(line, "\\303\\222", "Ò");
        boost::replace_all(line, "\\303\\223", "Ó");
        boost::replace_all(line, "\\303\\224", "Ô");
        boost::replace_all(line, "\\303\\225", "Õ");
        boost::replace_all(line, "\\303\\226", "Ö");
        boost::replace_all(line, "\\303\\227", "×");
        boost::replace_all(line, "\\303\\230", "Ø");
        boost::replace_all(line, "\\303\\231", "Ù");
        boost::replace_all(line, "\\303\\232", "Ú");
        boost::replace_all(line, "\\303\\233", "Û");
        boost::replace_all(line, "\\303\\234", "Ü");
        boost::replace_all(line, "\\303\\235", "Ý");
        boost::replace_all(line, "\\303\\236", "Þ");
        boost::replace_all(line, "\\303\\237", "ß");
        boost::replace_all(line, "\\303\\240", "à");
        boost::replace_all(line, "\\303\\241", "á");
        boost::replace_all(line, "\\303\\242", "â");
        boost::replace_all(line, "\\303\\243", "ã");
        boost::replace_all(line, "\\303\\244", "ä");
        boost::replace_all(line, "\\303\\245", "å");
        boost::replace_all(line, "\\303\\246", "æ");
        boost::replace_all(line, "\\303\\247", "ç");
        boost::replace_all(line, "\\303\\250", "è");
        boost::replace_all(line, "\\303\\251", "é");
        boost::replace_all(line, "\\303\\252", "ê");
        boost::replace_all(line, "\\303\\253", "ë");
        boost::replace_all(line, "\\303\\254", "ì");
        boost::replace_all(line, "\\303\\255", "í");
        boost::replace_all(line, "\\303\\256", "î");
        boost::replace_all(line, "\\303\\257", "ï");
        boost::replace_all(line, "\\303\\260", "ð");
        boost::replace_all(line, "\\303\\261", "ñ");
        boost::replace_all(line, "\\303\\262", "ò");
        boost::replace_all(line, "\\303\\263", "ó");
        boost::replace_all(line, "\\303\\264", "ô");
        boost::replace_all(line, "\\303\\265", "õ");
        boost::replace_all(line, "\\303\\266", "ö");
        boost::replace_all(line, "\\303\\267", "÷");
        boost::replace_all(line, "\\303\\270", "ø");
        boost::replace_all(line, "\\303\\271", "ù");
        boost::replace_all(line, "\\303\\272", "ú");
        boost::replace_all(line, "\\303\\273", "û");
        boost::replace_all(line, "\\303\\274", "ü");
        boost::replace_all(line, "\\303\\275", "ý");
        boost::replace_all(line, "\\303\\276", "þ");
        boost::replace_all(line, "\\303\\277", "ÿ");
    }
//     return line;
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


// Loads a saved model.
bool qnmt::LoadModel(const tensorflow::string& export_dir) 
{
  tensorflow::SessionOptions session_options;
  tensorflow::RunOptions run_options;

  tensorflow::Status load_saved_model_status =
      tensorflow::LoadSavedModel(session_options, run_options, export_dir,
                     {tensorflow::kSavedModelTagServe}, &_bundle);

  if (!load_saved_model_status.ok()) 
  {
    std::cerr << load_saved_model_status << std::endl;
    return false;
  }

  return true;
}

bool qnmt::LoadGraph(const std::string& graph_file) 
{
  tensorflow::Status load_graph_status = LoadGraph(graph_file, &_session);
  if (!load_graph_status.ok()) 
  {
    std::cerr << load_graph_status << std::endl;
    return false;
  }
  return true;
}

tensorflow::Status qnmt::LoadGraph(const string& graph_file_name, std::unique_ptr<tensorflow::Session>* session) 
{
  tensorflow::GraphDef graph_def;
  tensorflow::Status load_graph_status = ReadBinaryProto(tensorflow::Env::Default(), graph_file_name, &graph_def);
  if (!load_graph_status.ok()) 
  {
    return tensorflow::errors::NotFound("Failed to load compute graph at '", graph_file_name, "'");
  }
  session->reset(tensorflow::NewSession(tensorflow::SessionOptions()));
  tensorflow::Status session_create_status = (*session)->Create(graph_def);
  if (!session_create_status.ok()) 
  {
    return session_create_status;
  }
  return tensorflow::Status::OK();
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
std::vector<tensorflow::int32> qnmt::PadBatch(std::vector<std::vector<tensorflow::string> >& batch_tokens) 
{
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

// Tanslates a batch of tokenizes sentences.
bool qnmt::NMTBatch(
    std::vector<std::vector<tensorflow::string> > batch_tokens,
    std::vector<std::vector<tensorflow::string> >& output_batch_tokens,
    std::vector<float>& output_batch_scores
                   ) {
  // Pad batch.
  std::vector<tensorflow::int32> lengths = PadBatch(batch_tokens);
//   for (int l_i=0; l_i < (int)batch_tokens.size(); l_i++)
//   {
//      cerr << "Id " <<l_i << endl;
//      cerr << "Batch Content : " << Join_str(batch_tokens[l_i]," ") << endl;
//   }

  tensorflow::int64 batch_size = (long)batch_tokens.size();
  tensorflow::int64 max_length = (long)batch_tokens.front().size();
  
  std::vector<tensorflow::string> flat_batch_tokens = FlattenVector(batch_tokens);
  tensorflow::Tensor tokens_tensor = AsTensor(flat_batch_tokens, {batch_size, max_length});
  tensorflow::Tensor lengths_tensor = AsTensor(lengths);
//   cerr <<"Tokens: " << tokens_tensor.SummarizeValue(250) << endl;
//   cerr <<"Lenghts: "<< lengths_tensor.SummarizeValue(250) << endl;


  // Resolve name of inputs to fed and outputs to fetch.
  const auto signature_def_map = _bundle.meta_graph_def.signature_def();
  const auto signature_def = signature_def_map.at(tensorflow::kDefaultServingSignatureDefKey);
  const tensorflow::string tokens_input_name = signature_def.inputs().at("tokens").name();
  const tensorflow::string length_input_name = signature_def.inputs().at("length").name();
  const tensorflow::string tokens_output_name = signature_def.outputs().at("tokens").name();
  const tensorflow::string length_output_name = signature_def.outputs().at("length").name();
  const tensorflow::string probs_output_name = signature_def.outputs().at("log_probs").name();

  // Forward in the graph.
  std::vector<tensorflow::Tensor> outputs;
  tensorflow::Status run_status = _bundle.session->Run({{tokens_input_name, tokens_tensor}, {length_input_name, lengths_tensor}}, {tokens_output_name, length_output_name, probs_output_name}, {}, &outputs);

  if (!run_status.ok()) {
    std::cerr << "Running model failed: " << run_status << std::endl;
    return false;
  }
  for (int l_i=0; l_i < (int)outputs.size(); l_i++)
  {
     cerr << "Id " <<l_i << endl;
     cerr << "Content : " << outputs[l_i].SummarizeValue(500) << endl;
  }
  // Convert TensorFlow tensors to Eigen tensors.
  auto e_tokens = outputs[0].tensor<tensorflow::string,3>();
  auto e_length = outputs[1].matrix<tensorflow::int32>();
  auto e_probs = outputs[2].matrix<float>();
//  vector<long> e_length_shape;
//  for (long s = 0; s < batch_size; s++) 
//  {
//      e_length_shape.push_back(outputs[1].shape().dim_size(s));
//      cerr << "outputs[1].shape() " << outputs[1].shape() << endl;
//      cerr << "outputs[1].shape().dim_size(s) " << outputs[1].shape().dim_size(s) << endl;
//  }
    
  // Collect results in C++ vectors.
//   cerr << outputs[1].shape() << endl;
//   cerr << outputs[1].shape().dim_size(0) << endl;
//   cerr << outputs[1].shape().dim_size(1) << endl;
//   cerr << outputs[1].shape(1) << endl;
//  cerr << "Batch Size: " << batch_size << endl;
  long prev=0;
  for (long b = 0; b < e_length.dimension(0); b++) 
  {
//	cerr << "e_length_shape: " << e_length_shape[b] << endl;
      for (long nb = 0 ; nb < e_length.dimension(1); nb++)
      {
//	cerr << e_length.rank() << endl;
//        cerr << e_length.dimension(0) << endl;
//	cerr << e_length.dimension(1) << endl;
//      long len = e_length(b, 0);
//      std::vector<tensorflow::string> output_tokens;
//      output_tokens.reserve(len);
//      for (long i = 0; i < len - 1; ++i) 
//      {
//          output_tokens.push_back(e_tokens(b, 0, i));
//      }
//      output_batch_tokens.push_back(output_tokens);
        long len = e_length(b, nb);
//         cerr << "*******************" << endl;
//         cerr << prev << endl;
//         cerr << len << endl;
    //      cerr << vec_length[b] << endl;
        std::vector<tensorflow::string> output_tokens;
        output_tokens.reserve(len);
        for (long i = 0; i < len-1 ; ++i) 
        {
    //      cerr << vec_tokens[i] << endl;
    //         cerr << str_to_utf8(vec_tokens[i])<< endl;
            string tmp_token=e_tokens(b, nb, i);
            str_to_utf8(tmp_token);
            output_tokens.push_back(tmp_token);
        }
//         prev=len;
//         stringstream ss;
//         ss << e_probs(b, nb);
//         output_tokens.push_back(ss.str());
        if (b == 0) 
        {
            output_batch_tokens.push_back(output_tokens);
            output_batch_scores.push_back(e_probs(b, nb));
        }
        else 
        {
            output_batch_tokens.at(nb).insert(output_batch_tokens.at(nb).end(), output_tokens.begin(), output_tokens.end() );
            output_batch_scores.at(nb)=output_batch_scores.at(nb)+e_probs(b, nb);
        }
      }
  }
//   for (long b = 0; b < (long)output_batch_tokens.size(); b++) 
//   {
//       cerr << "Outputs : " << endl;
//       for (long nb = 0 ; nb < (long)output_batch_tokens[b].size(); nb++)
//       {
//           cerr << output_batch_tokens[b][nb] << " ";
//       }
//       cerr << endl;
//   }
  
  return true;
}

// Tanslates a batch of tokenizes sentences.
bool qnmt::NMTBatchGraph(
    std::vector<std::vector<tensorflow::string> > batch_tokens,
    std::vector<std::vector<tensorflow::string> >& output_batch_tokens,
    std::vector<float>& output_batch_scores) 
            {
  // Pad batch.
  std::vector<tensorflow::int32> lengths = PadBatch(batch_tokens);

  tensorflow::int64 batch_size = batch_tokens.size();
  tensorflow::int64 max_length = batch_tokens.front().size();
  
  std::vector<tensorflow::string> flat_batch_tokens = FlattenVector(batch_tokens);
  tensorflow::Tensor tokens_tensor = AsTensor(flat_batch_tokens, {batch_size, max_length});
  tensorflow::Tensor lengths_tensor = AsTensor(lengths);

  // Resolve name of inputs to fed and outputs to fetch.
//   const auto signature_def_map = _session-> _bundle.meta_graph_def.signature_def();
//   const auto signature_def = signature_def_map.at(tensorflow::kDefaultServingSignatureDefKey);
//   const tensorflow::string tokens_input_name = signature_def.inputs().at("tokens").name();
//   const tensorflow::string length_input_name = signature_def.inputs().at("length").name();
//   const tensorflow::string tokens_output_name = signature_def.outputs().at("tokens").name();
//   const tensorflow::string length_output_name = signature_def.outputs().at("length").name();
  const tensorflow::string tokens_input_name = "tokens";
  const tensorflow::string length_input_name = "length";
  const tensorflow::string tokens_output_name = "tokens";
  const tensorflow::string length_output_name = "length";

  // Forward in the graph.
  std::vector<tensorflow::Tensor> outputs;
  tensorflow::Status run_status = _session->Run({{tokens_input_name, tokens_tensor}, {length_input_name, lengths_tensor}}, {tokens_output_name, length_output_name}, {}, &outputs);

  if (!run_status.ok()) {
    std::cerr << "Running graph failed: " << run_status << std::endl;
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
    std::vector<std::vector<tensorflow::string> >& output_batch_tokens,
    std::vector<float>& output_batch_scores) 
            {
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
//         std::cout << "the " <<iter->first <<" result tensor[" << output_index << "] is:" << std::endl << tensor.SummarizeValue(250) << std::endl;
      }else {
        std::cerr << "the " <<iter->first <<" result tensor[" << output_index << 
              "] convert failed." << std::endl;
      }
      ++output_index;
    }
// 
  // Collect results in C++ vectors.
  
  long prev=0;
  vector<string> vec_length;
  vector<string> vec_tokens;
  Split_str(e_length.SummarizeValue(250).substr(1).substr(0,e_length.SummarizeValue(250).size()-1),vec_length, " ");
  Split_str(e_tokens.SummarizeValue(250),vec_tokens, " ");
  cerr << batch_size <<endl;
  for (long b = 0; b < batch_size; ++b) 
  {
    long len = prev+atol(vec_length[b].c_str());
// 	cerr << len << endl;
// 	cerr << vec_length[b] << endl;
    std::vector<tensorflow::string> output_tokens;
    output_tokens.reserve(len);
    for (long i = prev; i < len-1 ; ++i) 
    {
// 	cerr << vec_tokens[i] << endl;
//         cerr << str_to_utf8(vec_tokens[i])<< endl;
        str_to_utf8(vec_tokens[i]);
        output_tokens.push_back(vec_tokens[i]);
    }
    prev=len;
    output_batch_tokens.push_back(output_tokens);
  }  

// 	cerr << "Translation done... " << endl;
  return "Done.";
  } else {
    std::cerr << "gRPC call return code: " <<status.error_code() << ": " << status.error_message() << std::endl;
    return "gRPC failed.";
  }
 
  return true;
}
