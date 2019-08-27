// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "qtokenizer.h"
using namespace std;
using namespace qnlp;
qtokenizer::qtokenizer(string &lang)
{
    _lang = lang;
    if (lang == "fr")
    {
        _fr_tokenizer = new qnlp::Tokenizer_fr(qnlp::Tokenizer::PLAIN,true,false,false,false);
    }
    else if (lang == "en")
    {
        _en_tokenizer = new qnlp::Tokenizer_en(qnlp::Tokenizer::PLAIN,true,false,false,false);
    }
    else
    {
        _tokenizer = new qnlp::Tokenizer(qnlp::Tokenizer::PLAIN,true,false,false,false);
    }
}
void qtokenizer::set_tokenizer(string &lang)
{
    _lang = lang;
    if (_lang == "fr")
    {
        _fr_tokenizer = new qnlp::Tokenizer_fr(qnlp::Tokenizer::PLAIN,true,false,false,false);
    }
    else if (_lang == "en")
    {
        _en_tokenizer = new qnlp::Tokenizer_en(qnlp::Tokenizer::PLAIN,true,false,false,false);
    }
    else
    {
        _tokenizer = new qnlp::Tokenizer(qnlp::Tokenizer::PLAIN,true,false,false,false);
    }
}
qtokenizer::qtokenizer(string &lang, bool lowercase)
{
    _lang = lang;
    if (lang == "fr")
    {
        _fr_tokenizer = new qnlp::Tokenizer_fr(qnlp::Tokenizer::PLAIN,lowercase,false,false,false);
    }
    else if (lang == "en")
    {
        _en_tokenizer = new qnlp::Tokenizer_en(qnlp::Tokenizer::PLAIN,lowercase,false,false,false);
    }
    else
    {
        _tokenizer = new qnlp::Tokenizer(qnlp::Tokenizer::PLAIN,lowercase,false,false,false);
    }
}
void qtokenizer::set_tokenizer(string &lang, bool lowercase)
{
    _lang = lang;
    if (_lang == "fr")
    {
        _fr_tokenizer = new qnlp::Tokenizer_fr(qnlp::Tokenizer::PLAIN,lowercase,false,false,false);
    }
    else if (_lang == "en")
    {
        _en_tokenizer = new qnlp::Tokenizer_en(qnlp::Tokenizer::PLAIN,lowercase,false,false,false);
    }
    else
    {
        _tokenizer = new qnlp::Tokenizer(qnlp::Tokenizer::PLAIN,lowercase,false,false,false);
    }
}
vector<string> qtokenizer::tokenize(string &input)
{
    vector<string> to_return;
    if (_lang == "fr")
    {
        to_return=_fr_tokenizer->tokenize_sentence(input);
    }
    else if (_lang == "en")
    {
        to_return=_en_tokenizer->tokenize_sentence(input);
    }
    else
    {
        to_return=_tokenizer->tokenize_sentence(input);
    }
    return to_return;
}
string qtokenizer::tokenize_str(string &input)
{
    if (_lang == "fr")
    {
        return _fr_tokenizer->tokenize_sentence_to_string(input);
    }
    else if (_lang == "en")
    {
        return _en_tokenizer->tokenize_sentence_to_string(input);
    }
    else
    {
        return _tokenizer->tokenize_sentence_to_string(input);
    }
}


