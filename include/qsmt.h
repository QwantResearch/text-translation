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
