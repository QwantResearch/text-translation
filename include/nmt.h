// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#pragma once
#include <iostream>
#include <boost/locale.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "utils.h"
#include "easywsclient/easywsclient.h"


// #include "qsmt.h"


using namespace std;
using easywsclient::WebSocket;


class nmt
{
    public:
        nmt();
        nmt(std::string model_name_param, std::string& address_server);
        bool LoadModel(std::string model_name_param, std::string& address_server);
        bool NMTTranslate(std::string& sentence_to_translate, std::vector< std::string > translation_output, std::vector< float >& output_translation_scores, std::vector< std::vector< std::string > >& output_alignement_scores);
        bool getLocal();
    private:
      bool _local;
      string _model_name;
      string _address;
  
};
