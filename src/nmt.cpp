// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "nmt.h"

using namespace std;


void str_to_utf8(string& line)
{
    boost::replace_all(line, "\\'", "'");
    int line_size=(int)line.size();
    for (int i=0; i< line_size; i++)
//     while ((int)line.find("\\") > -1)
    {
        boost::replace_all(line, "\\302\\241", "¡");
        boost::replace_all(line, "\\302\\242", "¢");
        boost::replace_all(line, "\\302\\243", "£");
        boost::replace_all(line, "\\302\\244", "¤");
        boost::replace_all(line, "\\302\\245", "¥");
        boost::replace_all(line, "\\302\\246", "¦");
        boost::replace_all(line, "\\302\\247", "§");
        boost::replace_all(line, "\\302\\250", "¨");
        boost::replace_all(line, "\\302\\251", "©");
        boost::replace_all(line, "\\302\\252", "ª");
        boost::replace_all(line, "\\302\\253", "«");
        boost::replace_all(line, "\\302\\254", "¬");
        boost::replace_all(line, "\\302\\255", "­");
        boost::replace_all(line, "\\302\\256", "®");
        boost::replace_all(line, "\\302\\257", "¯");
        boost::replace_all(line, "\\302\\260", "°");
        boost::replace_all(line, "\\302\\261", "±");
        boost::replace_all(line, "\\302\\262", "²");
        boost::replace_all(line, "\\302\\263", "³");
        boost::replace_all(line, "\\302\\264", "´");
        boost::replace_all(line, "\\302\\265", "µ");
        boost::replace_all(line, "\\302\\266", "¶");
        boost::replace_all(line, "\\302\\267", "·");
        boost::replace_all(line, "\\302\\270", "¸");
        boost::replace_all(line, "\\302\\271", "¹");
        boost::replace_all(line, "\\302\\272", "º");
        boost::replace_all(line, "\\302\\273", "»");
        boost::replace_all(line, "\\302\\274", "¼");
        boost::replace_all(line, "\\302\\275", "½");
        boost::replace_all(line, "\\302\\276", "¾");
        boost::replace_all(line, "\\302\\277", "¿");
        boost::replace_all(line, "\\303\\200", "À");
        boost::replace_all(line, "\\303\\201", "Á");
        boost::replace_all(line, "\\303\\202", "Â");
        boost::replace_all(line, "\\303\\203", "Ã");
        boost::replace_all(line, "\\303\\204", "Ä");
        boost::replace_all(line, "\\303\\205", "Å");
        boost::replace_all(line, "\\303\\206", "Æ");
        boost::replace_all(line, "\\303\\207", "Ç");
        boost::replace_all(line, "\\303\\210", "È");
        boost::replace_all(line, "\\303\\211", "É");
        boost::replace_all(line, "\\303\\212", "Ê");
        boost::replace_all(line, "\\303\\213", "Ë");
        boost::replace_all(line, "\\303\\214", "Ì");
        boost::replace_all(line, "\\303\\215", "Í");
        boost::replace_all(line, "\\303\\216", "Î");
        boost::replace_all(line, "\\303\\217", "Ï");
        boost::replace_all(line, "\\303\\220", "Ð");
        boost::replace_all(line, "\\303\\221", "Ñ");
        boost::replace_all(line, "\\303\\222", "Ò");
        boost::replace_all(line, "\\303\\223", "Ó");
        boost::replace_all(line, "\\303\\224", "Ô");
        boost::replace_all(line, "\\303\\225", "Õ");
        boost::replace_all(line, "\\303\\226", "Ö");
        boost::replace_all(line, "\\303\\227", "×");
        boost::replace_all(line, "\\303\\230", "Ø");
        boost::replace_all(line, "\\303\\231", "Ù");
        boost::replace_all(line, "\\303\\232", "Ú");
        boost::replace_all(line, "\\303\\233", "Û");
        boost::replace_all(line, "\\303\\234", "Ü");
        boost::replace_all(line, "\\303\\235", "Ý");
        boost::replace_all(line, "\\303\\236", "Þ");
        boost::replace_all(line, "\\303\\237", "ß");
        boost::replace_all(line, "\\303\\240", "à");
        boost::replace_all(line, "\\303\\241", "á");
        boost::replace_all(line, "\\303\\242", "â");
        boost::replace_all(line, "\\303\\243", "ã");
        boost::replace_all(line, "\\303\\244", "ä");
        boost::replace_all(line, "\\303\\245", "å");
        boost::replace_all(line, "\\303\\246", "æ");
        boost::replace_all(line, "\\303\\247", "ç");
        boost::replace_all(line, "\\303\\250", "è");
        boost::replace_all(line, "\\303\\251", "é");
        boost::replace_all(line, "\\303\\252", "ê");
        boost::replace_all(line, "\\303\\253", "ë");
        boost::replace_all(line, "\\303\\254", "ì");
        boost::replace_all(line, "\\303\\255", "í");
        boost::replace_all(line, "\\303\\256", "î");
        boost::replace_all(line, "\\303\\257", "ï");
        boost::replace_all(line, "\\303\\260", "ð");
        boost::replace_all(line, "\\303\\261", "ñ");
        boost::replace_all(line, "\\303\\262", "ò");
        boost::replace_all(line, "\\303\\263", "ó");
        boost::replace_all(line, "\\303\\264", "ô");
        boost::replace_all(line, "\\303\\265", "õ");
        boost::replace_all(line, "\\303\\266", "ö");
        boost::replace_all(line, "\\303\\267", "÷");
        boost::replace_all(line, "\\303\\270", "ø");
        boost::replace_all(line, "\\303\\271", "ù");
        boost::replace_all(line, "\\303\\272", "ú");
        boost::replace_all(line, "\\303\\273", "û");
        boost::replace_all(line, "\\303\\274", "ü");
        boost::replace_all(line, "\\303\\275", "ý");
        boost::replace_all(line, "\\303\\276", "þ");
        boost::replace_all(line, "\\303\\277", "ÿ");
    }
//     return line;
}

nmt::nmt(std::string model_name_param, std::string& address_server)
{
    LoadModel(model_name_param,address_server);
}
nmt::nmt()
{
    cerr << "Warning: NMT object is empty" <<endl;
    _local=true;
}

bool nmt::getLocal()
{
    return _local;
}


bool nmt::LoadModel(std::string model_name_param, std::string& address_server)
{
    _model_name=model_name_param;
    _address=address_server;
    _local=false;
    return true;
}


bool nmt::NMTTranslate(std::string& sentence_to_translate, std::vector< std::vector< std::string > > translation_output, std::vector< float >& output_translation_scores, std::vector< std::vector< std::string > >& output_alignement_scores)
{
    std::unique_ptr<WebSocket> ws(WebSocket::from_url(_address));
    assert(ws);

    ws->send(sentence_to_translate);
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
    
    
    return true;

}



