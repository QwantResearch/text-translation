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
#include <iostream>

#include <tensorflow/cc/saved_model/loader.h>
#include <tensorflow/cc/saved_model/signature_constants.h>
#include <tensorflow/cc/saved_model/tag_constants.h>
#include <tensorflow/core/lib/io/path.h>
#include <tensorflow/core/platform/init_main.h>
#include <tensorflow/core/util/command_line_flags.h>

using namespace std;
// Displays a batch of tokens.
void PrintBatch(
    const std::vector<std::vector<tensorflow::string> >& batch_tokens) {
  for (const auto& tokens : batch_tokens) {
    for (const auto& token : tokens) {
      std::cout << " " << token;
    }
    std::cout << std::endl;
  }
}

// Pads a batch of tokens and returns the length of each sequence.
std::vector<tensorflow::int32> PadBatch(
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

// Loads a saved model.
bool LoadModel(tensorflow::SavedModelBundle& bundle,
               const tensorflow::string& export_dir) {
  tensorflow::SessionOptions session_options;
  tensorflow::RunOptions run_options;

  tensorflow::Status load_saved_model_status =
      LoadSavedModel(session_options, run_options, export_dir,
                     {tensorflow::kSavedModelTagServe}, &bundle);

  if (!load_saved_model_status.ok()) {
    std::cerr << load_saved_model_status << std::endl;
    return false;
  }

  return true;
}

// Tanslates a batch of tokenizes sentences.
bool TranslateBatch(
    tensorflow::SavedModelBundle& bundle,
    std::vector<std::vector<tensorflow::string> > batch_tokens,
    std::vector<std::vector<tensorflow::string> >& output_batch_tokens) {
  // Pad batch.
  std::vector<tensorflow::int32> lengths = PadBatch(batch_tokens);

  tensorflow::int64 batch_size = batch_tokens.size();
  tensorflow::int64 max_length = batch_tokens.front().size();
  std::vector<std::vector<tensorflow::string> > chars_list_bis;
  int l_inc=0;
  
//   length = len(tokens)
//   length_word = 0
//   int length_word
//   for word in tokens:
//       if length_word < len(list(word)):
//           length_word = len(list(word))
//       chars=[]
//       for char in list(word):
//           chars.append(char)
//       chars_list.append(chars)
//   l_inc=0
//   for chars in chars_list:
//       l_inc=len(chars)
//       if l_inc < length_word:
//           while l_inc < length_word:
//               chars.append('')
//               l_inc = l_inc + 1
// 
//     l_inc=0
//   while (l_inc < length_word)
//   {
//       std::vector<tensorflow::string> l_vectmp;
//       chars_list_bis.push_back(l_vectmp);
//       int l_inc_bis=0
//       while (l_inc_bis < length)
//           chars_list_bis[l_inc].append(chars_list[l_inc_bis][l_inc])
//       
//           l_inc_bis=l_inc_bis+1
//       l_inc=l_inc+1
//   }

  // Convert to tensors.
  std::vector<tensorflow::string> flat_batch_tokens =
      FlattenVector(batch_tokens);
//   std::vector<std::vector<tensorflow::string>> flat_batch_char =
//       FlattenVector(batch_tokens);
  tensorflow::Tensor tokens_tensor =
      AsTensor(flat_batch_tokens, {batch_size, max_length});
  tensorflow::Tensor chars_tensor =
      AsTensor(flat_batch_tokens, {batch_size, max_length});
  tensorflow::Tensor lengths_tensor = AsTensor(lengths);

  // Resolve name of inputs to fed and outputs to fetch.
  const auto signature_def_map = bundle.meta_graph_def.signature_def();
  const auto signature_def = signature_def_map.at(
      tensorflow::kDefaultServingSignatureDefKey);
  const tensorflow::string tokens_input_name =
      signature_def.inputs().at("tokens").name();
//   const tensorflow::string char_input_name =
//       signature_def.inputs().at("char").name();
  const tensorflow::string length_input_name =
      signature_def.inputs().at("length").name();
  const tensorflow::string tokens_output_name =
      signature_def.outputs().at("tokens").name();
  const tensorflow::string length_output_name =
      signature_def.outputs().at("length").name();

  // Forward in the graph.
  std::vector<tensorflow::Tensor> outputs;
  tensorflow::Status run_status = bundle.session->Run(
      {{tokens_input_name, tokens_tensor}, {length_input_name, lengths_tensor}},
      {tokens_output_name, length_output_name}, {}, &outputs);

  if (!run_status.ok()) {
    std::cerr << "Running model failed: " << run_status << std::endl;
    return false;
  }

  // Convert TensorFlow tensors to Eigen tensors.
  auto e_tokens = outputs[0].tensor<tensorflow::string, 3>();
  auto e_length = outputs[1].matrix<tensorflow::int32>();

  // Collect results in C++ vectors.
  for (long b = 0; b < batch_size; ++b) {
    long len = e_length(b, 0);
    std::vector<tensorflow::string> output_tokens;
    output_tokens.reserve(len);
    for (long i = 0; i < len - 1; ++i) {
      output_tokens.push_back(e_tokens(b, 0, i));
    }
    output_batch_tokens.push_back(output_tokens);
  }

  return true;
}

int main(int argc, char* argv[]) {
  tensorflow::string export_dir;

  std::vector<tensorflow::Flag> flag_list = {tensorflow::Flag(
      "export_dir", &export_dir, "directory containing the exported model")};

  tensorflow::string usage = tensorflow::Flags::Usage(argv[0], flag_list);
  const bool parse_result = tensorflow::Flags::Parse(&argc, argv, flag_list);
  if (!parse_result) {
    std::cerr << usage << std::endl;
    return 1;
  }
  cerr << "Export Dir : "  << export_dir << endl;
  tensorflow::port::InitMain(argv[0], &argc, &argv);

  tensorflow::SavedModelBundle bundle;
  std::vector<std::vector<tensorflow::string> > output_batch_tokens;
  std::vector<std::vector<tensorflow::string> > batch_tokens = {
      {"Bonjour", "tout", "le", "monde", "!"},
      {"C'", "est", "un", "test", "."},
      {"Mon", "nom", "est", "Q@@", "want", "Research", "."}};

  if (!LoadModel(bundle, export_dir)) {
    return 1;
  }

  std::cout << "Input:" << std::endl;
  PrintBatch(batch_tokens);

  if (!TranslateBatch(bundle, batch_tokens, output_batch_tokens)) {
    return 1;
  }

  std::cout << "Output:" << std::endl;
  PrintBatch(output_batch_tokens);

  return 0;
}