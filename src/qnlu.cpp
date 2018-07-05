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
#include "qnlu.h"

using namespace std;

qnlu::qnlu(const tensorflow::string& export_dir)
{
    LoadModel(export_dir);
}
qnlu::qnlu()
{
    cerr << "Warning: NLU object is empty" <<endl;
}


// Displays a batch of tokens.
void qnlu::PrintBatch(
    const std::vector<std::vector<tensorflow::string> >& batch_tokens) {
  for (const auto& tokens : batch_tokens) {
    for (const auto& token : tokens) {
      std::cout << " " << token;
    }
    std::cout << std::endl;
  }
}

// Pads a batch of tokens and returns the length of each sequence.
std::vector<tensorflow::int32> qnlu::PadBatch(
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
bool qnlu::LoadModel(const tensorflow::string& export_dir) {
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
bool qnlu::NLUBatch(
    std::vector<std::vector<tensorflow::string> > batch_tokens,
    std::vector<std::vector<tensorflow::string> >& output_batch_tokens) {
  // Pad batch.
  std::vector<tensorflow::int32> lengths = PadBatch(batch_tokens);

  tensorflow::int64 batch_size = batch_tokens.size();
  tensorflow::int64 max_length = batch_tokens.front().size();
  
  std::vector<std::vector<std::vector<tensorflow::string> > > chars_list_bis_batch;
  std::vector<std::vector<tensorflow::string> > chars_list;
  std::vector<tensorflow::string> l_tokens_in_batch ;
  
  int l_inc=0;
  int l_inc_batch=0;
  int l_inc_char=0;
  
  int length = 0;
  int max_length_word = 0;
  for (l_inc_batch = 0; l_inc_batch < batch_size; l_inc_batch++)
  {
      l_tokens_in_batch = batch_tokens.at(l_inc_batch);
      length = l_tokens_in_batch.size();
      for (l_inc = 0 ; l_inc < length; l_inc++)
      {
          string word = l_tokens_in_batch.at(l_inc);
          if (max_length_word < (int)word.size())
          {
              max_length_word = (int)word.size();
          }
      }
  }
  for (l_inc_batch = 0; l_inc_batch < batch_size; l_inc_batch++)
  {
      l_tokens_in_batch = batch_tokens.at(l_inc_batch);
      length = l_tokens_in_batch.size();
      std::vector<std::vector<tensorflow::string> > chars_list_bis;
      for (l_inc = 0 ; l_inc < length; l_inc++)
      {
          vector<tensorflow::string> l_char;
          tensorflow::string word = l_tokens_in_batch.at(l_inc);
          for (l_inc_char = 0 ; l_inc_char < max_length_word; l_inc_char++)
          {
              if (l_inc_char < (int)word.size())
              {
                  stringstream l_ss;
                  l_ss << word[l_inc_char];
                  l_char.push_back(l_ss.str());
              }
              else
              {
                  l_char.push_back("");
              }
          }
          chars_list.push_back(l_char);
      }
 

    l_inc=0;  
    while (l_inc < max_length_word)
    {
        std::vector<tensorflow::string> l_vectmp;
        chars_list_bis.push_back(l_vectmp);
        int l_inc_bis=0;
        while (l_inc_bis < length)
        {
            chars_list_bis[l_inc].push_back(chars_list[l_inc_bis][l_inc]);
            l_inc_bis=l_inc_bis+1;
        }
        l_inc=l_inc+1;
    }
    chars_list_bis_batch.push_back(chars_list_bis);
  }

  // Convert to tensors.
  std::vector<tensorflow::string> flat_batch_tokens = FlattenVector(batch_tokens);
  std::vector<tensorflow::string> flat_batch_char = FlattenVector(chars_list_bis_batch);
  tensorflow::Tensor tokens_tensor  =  AsTensor(flat_batch_tokens, {batch_size, max_length});
  tensorflow::Tensor chars_tensor   = AsTensor(flat_batch_char, {batch_size, max_length, max_length_word});
  tensorflow::Tensor lengths_tensor = AsTensor(lengths);

  // Resolve name of inputs to fed and outputs to fetch.
  const auto signature_def_map = bundle.meta_graph_def.signature_def();
  const auto signature_def = signature_def_map.at(tensorflow::kDefaultServingSignatureDefKey);
  const tensorflow::string tokens_input_name  = signature_def.inputs().at("tokens").name();
  const tensorflow::string char_input_name    = signature_def.inputs().at("chars").name();
  const tensorflow::string length_input_name  = signature_def.inputs().at("length").name();
  const tensorflow::string tokens_output_name = signature_def.outputs().at("tags").name();
  const tensorflow::string length_output_name = signature_def.outputs().at("length").name();

  // Forward in the graph.
  std::vector<tensorflow::Tensor> outputs;
  tensorflow::Status run_status = bundle.session->Run(
      {{tokens_input_name, tokens_tensor}, {char_input_name, chars_tensor}, {length_input_name, lengths_tensor}},
      {tokens_output_name, length_output_name}, {}, &outputs);

  if (!run_status.ok()) {
    std::cerr << "Running model failed: " << run_status << std::endl;
    return false;
  }

  // Convert TensorFlow tensors to Eigen tensors.
  auto e_tags = outputs[0].tensor<tensorflow::string,2>();
  auto e_length = outputs[1].tensor<tensorflow::int32,1>();
// 
  // Collect results in C++ vectors.
  for (long b = 0; b < batch_size; ++b) {
    long len = e_length(b);
    std::vector<tensorflow::string> output_tokens;
    output_tokens.reserve(len);
    for (long i = 0; i < len ; ++i) {
      output_tokens.push_back(e_tags(b, i));
    }
    output_batch_tokens.push_back(output_tokens);
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

  if (!NLUBatch(batch_tokens, output_batch_tokens)) {
    return 1;
  }

  std::cout << "Output:" << std::endl;
  PrintBatch(output_batch_tokens);

  return 0;
}*/