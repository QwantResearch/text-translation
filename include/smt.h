// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#pragma once
#include <iostream>
#include <cstdlib>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include "utils.h"

using namespace std;

class qsmt
{
    public:
        qsmt();
        qsmt(std::string& address, std::string& iport, std::string& route, std::string& name);
        bool SMTBatch(std::string& tokens, std::string& output_tokens, float& output_scores);
        bool NMTBatchSMT(vector< vector< string > > batch_tokens, vector< vector< string > >& output_batch_tokens, vector< float >& output_batch_scores);
        bool getLocal();
        void setqsmt(std::string& address, std::string& iport, std::string& route, std::string& name);
    private:
      std::string _address;
      std::string _iport;
      std::string _route;
      bool _local;
      std::string _model_name;
  
};
