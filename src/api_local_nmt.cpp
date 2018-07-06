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
#include <algorithm>

#include <iostream>
#include <pistache/http.h>
#include <pistache/client.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <nlohmann/json.hpp>
#include <map>
#include <fasttext/fasttext.h>
#include <sstream>
#include "qtokenizer.h"
#include "qnmt.h"
#include "utils.h"
#include "bpe.h"

using namespace std;
using namespace nlohmann;
using namespace fasttext;
using namespace Pistache;

void printCookies(const Http::Request& req) {
    auto cookies = req.cookies();
    std::cout << "Cookies: [" << std::endl;
    const std::string indent(4, ' ');
    for (const auto& c: cookies) {
        std::cout << indent << c.name << " = " << c.value << std::endl;
    }
    std::cout << "]" << std::endl;
}

namespace Generic {

void handleReady(const Rest::Request&, Http::ResponseWriter response) {
    response.send(Http::Code::Ok, "1");
}

}

class Classifier {
public:
    Classifier(std::string filename, string domain)
    {
        _model.loadModel(filename.c_str());
        _domain = domain;
    }
    
    vector < pair < fasttext::real, string > > prediction(string text, int count)
    {
        vector <pair <fasttext::real, string> > results;
        istringstream istr(text);
        _model.predict(istr,count,results);
        for (int i=0;i<(int)results.size();i++)
        {
            results.at(i).first=exp(results.at(i).first);
            results.at(i).second=results.at(i).second.substr(9);
        }
        return results;
    }
    std::string getDomain()
    {
        return _domain;
    }

private:
    std::string _domain;
    fasttext::FastText _model;
};

class NMT {
public:
    std::string _domain;
    std::string _src;
    std::string _tgt;

    NMT(std::string dirname, string domain, string src, string tgt, string bpe)
    {
        _model.LoadModel(dirname);
        _domain = domain;
        _bpe = new BPE(bpe);
        _src=src;
        _tgt=tgt;
    }
    
    bool batch_NMT(vector<vector<string>>& input, vector<vector<string>>& output)
    {
        return _model.NMTBatch(input,output);
    }
    std::string getDomain()
    {
        return _domain;
    }


private:
    qnmt _model;
    BPE* _bpe;
};


class StatsEndpoint {
public:
    StatsEndpoint(Address addr, string NMT_config)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    { 
        ifstream model_config;
        string line;
//         model_config.open(classif_config);
// //         Classifier * l_classif;
//         while (getline(model_config,line))
//         {
//             string domain=line.substr(0,line.find("\t"));
//             string file=line.substr(line.find("\t")+1);
//             cerr << domain <<"\t"<< file << endl;
//             _list_classifs.push_back(new Classifier(file,domain));
//         }
//         model_config.close();
        model_config.open(NMT_config);
//         Classifier * l_classif;
        while (getline(model_config,line))
        {
            vector<string> vec_line;
            Split(line,vec_line,"\t");
//             cerr << vec_line.size() << endl;
            string src=vec_line.at(0);
            string tgt=vec_line.at(1);
            string bpe=vec_line.at(2);
            string file=vec_line.at(3);
            string domain=vec_line.at(4);
//             cerr << src <<"\t"<< tgt<< "\t" << bpe<< endl;
//             cerr << domain <<"\t"<< file << endl;
            _list_nmt.push_back(new NMT(file,domain,src,tgt,bpe));
        }
        model_config.close();
        
      
    }

    void init(size_t thr = 2) 
    {
        auto opts = Http::Endpoint::options()
            .threads(thr)
            .flags(Tcp::Options::InstallSignalHandler);
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start() 
    {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

    void shutdown() 
    {
        httpEndpoint->shutdown();
    }
    

private:
    vector<Classifier*> _list_classifs;
    vector<NMT*> _list_nmt;
    
    void setupRoutes() {
        using namespace Rest;
        Routes::Post(router, "/translate/", Routes::bind(&StatsEndpoint::doNMTPost, this));
        Routes::Get(router, "/translate/", Routes::bind(&StatsEndpoint::doNMTGet, this));
        Routes::Get(router, "/languages/", Routes::bind(&StatsEndpoint::doNMTLanguagesGet, this));

    }
//     void doClassificationGet(const Rest::Request& request, Http::ResponseWriter response) {
//         response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
//         response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
//         response.send(Pistache::Http::Code::Ok, "{\"message\":\"success\"}");
//     }
//     void doClassificationDomainsGet(const Rest::Request& request, Http::ResponseWriter response) {
//         response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
//         response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
//         response.send(Pistache::Http::Code::Ok, "{\"domains\":[\"language_identification\",\"shopping\",\"advertisement\",\"iot\",\"intention\"]}");
//     }
    void doNMTLanguagesGet(const Rest::Request& request, Http::ResponseWriter response) {
        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        string response_str("");
        for (int i=0; i<(int) _list_nmt.size(); i++)
        {
            if ((int)response_str.length() != 0) response_str.append(",");
            response_str.append(_list_nmt.at(i)->_src);
            response_str.append("-");
            response_str.append(_list_nmt.at(i)->_tgt);
            response_str.append("-");
            response_str.append(_list_nmt.at(i)->_domain);
        }
        response_str="{\""+response_str+"\"]}";
        response.send(Pistache::Http::Code::Ok, response_str);
    }
//     void doClassificationPost(const Rest::Request& request, Http::ResponseWriter response) 
//     {
//         response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
//         nlohmann::json j = nlohmann::json::parse(request.body());
//         int count=10;
//         bool debugmode = false;
//         if (j.find("count") != j.end())
//         {
//             count=j["count"]; 
//         }
//         if (j.find("debug") != j.end())
//         {
//             debugmode=j["debug"]; 
//         }
//         if (j.find("text") != j.end())
//         {
//             string text=j["text"]; 
//             string lang=j["language"]; 
//             qtokenizer l_tok(lang,false);
//             
//             
//             j.push_back( nlohmann::json::object_t::value_type(string("tokenized"), l_tok.tokenize_str(text) ));
//             if (j.find("domain") != j.end())
//             {
//                 string domain=j["domain"]; 
//                 string tokenized=j["tokenized"]; 
//                 istringstream istr(tokenized);
//                 std::vector < std::pair < fasttext::real, std::string > > results;
//                 results = askClassification(tokenized,domain,count);
//                 j.push_back( nlohmann::json::object_t::value_type(string("intention"), results));
//             }
//             std::string s=j.dump();
//             response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
//             response.send(Http::Code::Ok, std::string(s));
//         }
//         response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
// //         response.headers().add<Http::Header::ContentType>(MIME('Powered-By', 'Qwant Research'));
//         response.send(Http::Code::Bad_Request, std::string("Missing things..."));
//     }    
    
    std::vector < std::pair < fasttext::real, std::string > > askClassification(std::string text, std::string domain, int count)
    {
        std::vector < std::pair < fasttext::real, std::string > > to_return;
        istringstream istr(text);
        auto it_classif = std::find_if(_list_classifs.begin(), _list_classifs.end(), [&](Classifier* l_classif) 
        {
            return l_classif->getDomain() == domain;
        }); 
        if (it_classif != _list_classifs.end())
        {
            to_return = (*it_classif)->prediction(text,count);
        }
        return to_return;
    }
    bool askNMT(vector<vector<string> > &input, json &output, string &domain, string &src, string &tgt, bool debugmode)
    {
        vector<vector<string> > result_batched ;
//         cerr << "Ze Size " << _list_nmt.size() << endl;
        auto it_nmt = std::find_if(_list_nmt.begin(), _list_nmt.end(), [&](NMT* l_nmt) 
        {
//             cerr << l_nmt->_domain <<"\t"<< domain << endl;
//             cerr << l_nmt->_src <<"\t"<< src << endl;
//             cerr << l_nmt->_tgt <<"\t"<< tgt << endl;
            return (l_nmt->_domain == domain && l_nmt->_src == src && l_nmt->_tgt == tgt) ;
        }); 
        if (it_nmt != _list_nmt.end())
        {
//             cerr << "Translation" << endl;
            (*it_nmt)->batch_NMT(input,result_batched);
        }
        string  translation_concat("");
        string  curr_token("");
        string  word_concat("");
//         cerr << result_batched.size() << endl;                
        for (int i=0;i<(int)result_batched.size();i++)
        {
//             cerr << result_batched.at(i).size() << endl;                
            for (int j=0;j<(int)result_batched.at(i).size();j++)
            {
                json j_tmp;
                curr_token=result_batched.at(i).at(j);
//                 cerr << curr_token << endl;
                int sep_pos = (int)curr_token.find("@@");
                if (sep_pos > -1)
                {
                    curr_token=curr_token.substr(0,sep_pos);
                }
                else
                {
                    if (translation_concat.length() > 0) translation_concat.append(" ");
                }
                translation_concat.append(result_batched.at(i).at(j));
            }
        }
        output.push_back(nlohmann::json::object_t::value_type(string("translation"), translation_concat));
        
    }
    
    void doNMTGet(const Rest::Request& request, Http::ResponseWriter response) {
        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        response.send(Pistache::Http::Code::Ok, "{\"message\":\"success\"}");
    }
    void doNMTPost(const Rest::Request& request, Http::ResponseWriter response) 
    {
        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        nlohmann::json j = nlohmann::json::parse(request.body());
        int count=10;
        bool debugmode = false;
        
        if (j.find("src") == j.end())
        {
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Bad_Request, "{\"Error\":\"parameter src is needed\"}");
        }

        if (j.find("tgt") == j.end())
        {
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Bad_Request, "{\"Error\":\"parameter tgt is needed\"}");
        }
        if (j.find("domain") == j.end())
        {
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Bad_Request, "{\"Error\":\"parameter domain is needed\"}");
        }
        if (j.find("count") != j.end())
        {
            count=j["count"]; 
        }
        if (j.find("debug") != j.end())
        {
            debugmode=j["debug"]; 
        }
        if (j.find("text") != j.end())
        {
            string text=j["text"]; 
            string src=j["src"]; 
            string tgt=j["tgt"]; 
            qtokenizer l_tok(src,false);
            
            
            j.push_back( nlohmann::json::object_t::value_type(string("tokenized"), l_tok.tokenize_str(text) ));
            vector<string> tokenized_vec = l_tok.tokenize(text) ;
            vector<vector<string> > tokenized_batched ;
            vector<string> line_tokenized;
            for (int l_inc=0; l_inc < (int)tokenized_vec.size(); l_inc++)
            {
                line_tokenized.push_back(tokenized_vec[l_inc]);
                if (l_inc == (int)tokenized_vec.size()-1 || ((int)tokenized_vec[l_inc].size() == 1 && (tokenized_vec[l_inc].compare(".")==0 || tokenized_vec[l_inc].compare("\n")==0)))
                {
                    tokenized_batched.push_back(line_tokenized);
                    line_tokenized.clear();
                }
            }
//             cerr << tokenized_batched.size() << endl;
            if (debugmode) j.push_back( nlohmann::json::object_t::value_type(string("tokenized_vec"), tokenized_batched ));
            if (j.find("domain") != j.end())
            {
                string domain=j["domain"]; 
                string tokenized=j["tokenized"]; 
//                 istringstream istr(tokenized);

//                 if (domain.find("iot")==0)
//                 {
//                     
//                     std::vector < std::pair < fasttext::real, std::string > > results = askClassification(tokenized,domain,count);
//                     j.push_back( nlohmann::json::object_t::value_type(string("intention"), results));
//                 }
//                 cerr << "Before asking" << endl;
                askNMT(tokenized_batched,j,domain,src,tgt,debugmode);
//                 cerr << "After asking" << endl;
            }
        }
        else
        {
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Bad_Request, "{\"Error\":\"parameter text is needed\"}");
        }
        std::string s=j.dump();
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, std::string(s));
    }
    void writeLog(string text_to_log)
    {
        
    }


    void doAuth(const Rest::Request& request, Http::ResponseWriter response) {
        printCookies(request);
        response.cookies()
            .add(Http::Cookie("lang", "fr-FR"));
        response.send(Http::Code::Ok);
    }

    typedef std::mutex Lock;
    typedef std::lock_guard<Lock> Guard;
    

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main(int argc, char *argv[]) {
    Port port(9009);

    int thr = 8;
//     string model_config_classif("model_classif_config.txt");
    string model_config_NMT("model_nmt_config.txt");
    if (argc >= 2) 
    {
        port = std::stol(argv[1]);
        if (argc >= 3)
        {
            thr = std::stol(argv[2]);
            if (argc >= 4)
            {
//                 model_config_classif = string(argv[3]);
//                 if (argc >= 5)
//                 {
                    model_config_NMT = string(argv[3]);
//                 }
            }
        }
    }

    Address addr(Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;
    

    StatsEndpoint stats(addr,model_config_NMT);

    stats.init(thr);
    stats.start();
    stats.shutdown();
}
