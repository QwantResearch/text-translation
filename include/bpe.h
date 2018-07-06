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
#pragma once
#include "processor.h"

#include <vector>
#include <string>
#include <string.h>
#include <fstream>
#include <set>
#include <unordered_map>
#include <iterator>
#include <algorithm>

template<class T>
inline void hash_combine(std::size_t & seed, const T & v)
{
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std
{
  template<typename S, typename T> struct hash<pair<S, T>>
  {
      inline size_t operator()(const pair<S, T> & v) const
      {
            size_t seed = 0;
            ::hash_combine(seed, v.first);
            ::hash_combine(seed, v.second);
            return seed;
          }
    };
}

class BPE : public Processor {
  using BPEPair = std::pair<std::string, std::string>;

  public:
    BPE();
    BPE(std::ifstream&& file, const std::string sep = "@@");

    BPE(const std::string& path, const std::string sep = "@@");

    std::vector<std::string> Segment(const std::string& sentence);

    void PrintSegment(const std::string& sentence);

    std::vector<std::string>& Encode(const std::string& word);

    std::vector<std::vector<std::string>> Encode(const std::vector<std::string>& words);

    std::vector<std::vector<std::string>> Preprocess(const std::vector<std::string> input);
    std::vector<std::string> Postprocess(const std::vector<std::string> input);

    virtual ~BPE() {}
  private:
    std::set<BPEPair> GetPairs(const std::vector<std::string>& word);

    const BPEPair* FindBestBigram(const std::set<BPEPair>& pairs);

    bool IsCached(const std::string& word) const;

    std::vector<std::string> SplitWordIntoLetters(const std::string& word) const;

    bool EndsWith(const std::string& fullString, const std::string suffix) const;

    std::unordered_map<BPEPair, size_t> bpeCodes_;
    const std::string sep_;

    std::unordered_map<std::string, std::vector<std::string>> cache_;


};
