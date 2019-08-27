// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "easywsclient/easywsclient.h"
#include <assert.h>
#include "utils.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <memory>

using namespace std;
using easywsclient::WebSocket;


string send_translation (string & to_translate, string & server_address)
{
    std::unique_ptr<WebSocket> ws(WebSocket::from_url(server_address));
    assert(ws);
//     ws->send("▁C ' ▁est ▁un ▁test ▁!");
    ws->send(to_translate);
    std::string message;
    while (ws->getReadyState() != WebSocket::CLOSED) {
        bool gotMessage = false;
        ws->poll();
        ws->dispatch([gotMessageOut=&gotMessage, messageOut=&message, ws=ws.get()](const std::string& message) {
            *gotMessageOut = true;
            *messageOut = message;
        });
        if (gotMessage) {
            ws->close();
            break;
        }
    }
    vector<string> full_hypothesis;
    Split(message,full_hypothesis,"\n");
    int inc_hyp=0;
    std::vector< std::vector< std::string > > translation_output; 
    std::vector< float > output_translation_scores;
    std::vector< std::vector< std::string > > output_alignement_scores;
    
    while (inc_hyp < (int)full_hypothesis.size())
    {
        vector<string> data;
        Split(full_hypothesis[inc_hyp],data,"||| ");
        std::vector< std::string > l_sentence;
        std::vector< std::string > l_alignement_scores;
        int inc_data=1;
        Split(data[inc_data],l_sentence," ");
        translation_output.push_back(l_sentence);
        inc_data++;
        if ((int)data.size() > 4)
        {
            Split(data[inc_data],l_alignement_scores," ");
            output_alignement_scores.push_back(l_alignement_scores);
            inc_data++;
        }
        inc_data++; // iterationg to avoid F-score.        
        output_translation_scores.push_back(atof(data[inc_data].c_str()));
        inc_hyp++;
    }
    
    return message;
}


int main()
{
    string server="ws://localhost:8081/translate";
    string message_to_translate="▁This ▁is ▁a ▁test ▁!";
    cout << send_translation (message_to_translate,server) << endl;
    return 0;
}
