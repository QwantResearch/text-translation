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
    return to_return;
}
