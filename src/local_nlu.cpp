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

#include "qnlu.h"
#include "qtokenizer.h"

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
  qnlu l_nlu(export_dir);
  std::vector<std::vector<tensorflow::string> > output_batch_tokens;
  std::vector<std::vector<tensorflow::string> > batch_tokens;
  string lang = "fr";
  qtokenizer l_tok(lang);
  std::vector<tensorflow::string> lines = {{"bonjour je voudrais réserver un hôtel à Paris"}, {"Quel est le taux de pollution à Lyon ?!"}};
  for (int l_inc=0; l_inc<lines.size(); l_inc++)
  {
      
      batch_tokens.push_back(l_tok.tokenize(lines[l_inc]));
  }
  std::cout << "Input:" << std::endl;
  l_nlu.PrintBatch(batch_tokens);

  if (!l_nlu.NLUBatch(batch_tokens, output_batch_tokens)) {
    return 1;
  }

  std::cout << "Output:" << std::endl;
  l_nlu.PrintBatch(output_batch_tokens);

  return 0;
}