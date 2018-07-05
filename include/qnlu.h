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

class qnlu
{
    public:
        qnlu();
        qnlu(const tensorflow::string&  filename);
        void PrintBatch(const std::vector<std::vector<tensorflow::string> >& batch_tokens);
        std::vector<tensorflow::int32> PadBatch(std::vector<std::vector<tensorflow::string> >& batch_tokens);
        bool LoadModel(const tensorflow::string& export_dir);
        bool NLUBatch(std::vector<std::vector<tensorflow::string> > batch_tokens, std::vector<std::vector<tensorflow::string> >& output_batch_tokens);
    private:
      tensorflow::SavedModelBundle bundle;
  
};
