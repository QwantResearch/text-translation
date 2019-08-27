// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include <iostream>

#include "qnlp/tokenizer.h"
#include "qnlp/en_tokenizer.h"
#include "qnlp/fr_tokenizer.h"

using namespace std;
using namespace qnlp;

class qtokenizer
{
    public:
        qnlp::Tokenizer * _tokenizer;
        qnlp::Tokenizer_fr * _fr_tokenizer;
        qnlp::Tokenizer_en * _en_tokenizer;
        string _lang;
        qtokenizer(string &lang);
        void set_tokenizer(string &lang);
        qtokenizer(string &lang, bool lowercase);
        void set_tokenizer(string &lang, bool lowercase);
        vector<string> tokenize(string &input);
        string tokenize_str(string &input);
};
