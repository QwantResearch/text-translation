// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "spm.h"


spm::spm(std::string& modelpath)
{
    const sentencepiece::util::Status status = _processor.Load(modelpath);
    if (!status.ok()) 
    {
        std::cerr << "Error while loading SentencePiece model: " << modelpath << std::endl;
        exit(1);
    }
}

std::vector< std::string > spm::segment(std::string& sentence)
{
    std::vector<std::string> to_return;
    _processor.Encode(sentence, &to_return);
    return to_return;
}

std::string spm::decode(std::vector<std::string>& vec_sentence)
{
    std::string to_return;
    const char specialChar[] = "\xe2\x96\x81";
    _processor.Decode(vec_sentence, &to_return);
    while((int)to_return.find(specialChar)>-1)
    {
        to_return=to_return.replace((int)to_return.find(specialChar),(int)strlen(specialChar)," ");
    }
    if (to_return[0]==' ')
    {
        to_return=to_return.substr(1,(int)to_return.length()-1);
    }
    return to_return;
}
