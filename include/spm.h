// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#pragma once
#include <sentencepiece_processor.h>
#include <sentencepiece_trainer.h>

#include <vector>
#include <iostream>
#include <string>
#include <string.h>
#include <fstream>
#include <set>
#include <unordered_map>
#include <iterator>
#include <algorithm>


class spm {

  public:
    spm(std::string& modelpath);
    std::vector<std::string> segment(std::string& sentence);
    std::string decode(std::vector<std::string> & sentence);
    
    
  private:
    sentencepiece::SentencePieceProcessor _processor;
};
