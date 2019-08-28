// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#pragma once
#include <iostream>
#include <boost/locale.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "utils.h"
#include "easywsclient/easywsclient.h"


#include "spm.h"
#include "tokenizer.h"


using namespace std;
using easywsclient::WebSocket;


class nmt
{
    public:
        nmt();
        ~nmt(){delete(_spm_src);delete(_spm_tgt);};
        nmt(std::string& model_name_param, std::string& address_server, std::string& spm_file_model, std::string& lang_src, std::string& lang_tgt);
        bool LoadModel(std::string model_name_param, std::string& address_server, std::string& spm_file_model, std::string& lang_src, std::string& lang_tgt);
        bool NMTTranslate(std::string& sentence_to_translate, std::vector< std::string >& translation_output, std::vector< float >& output_translation_scores, std::vector< std::string >& output_alignement_scores);
        bool NMTTranslateBatch(std::vector< std::string >& batch_sentence_to_translate, std::vector< std::string >& translation_output, std::vector< float >& output_translation_scores, std::vector< std::string >& output_alignement_scores);
        bool getLocal();
        std::string getDomain(){return _model_name;};
        std::string getLangSrc(){return _lang_src;};
        std::string getLangTgt(){return _lang_tgt;};
        std::vector <std::string> tokenize(std::string &input);
        std::string tokenize_str(std::string &input);
        std::string spm_segment(std::string &input);
        std::string detokenize_str(std::string& input);
        std::string spm_detokenize_str(std::string& input);
        
    private:
      bool _local;
      spm * _spm_src;
      spm * _spm_tgt;
      string _model_name;
      string _address;
      string _lang_src;
      string _lang_tgt;
      
  
};
