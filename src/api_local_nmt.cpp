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
    NMT(std::string dirname, string domain)
    {
        _model.LoadModel(dirname);
        _domain = domain;
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
    std::string _domain;
    qnlu _model;
};


class StatsEndpoint {
public:
    StatsEndpoint(Address addr, string classif_config, string NMT_config)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    { 
        ifstream model_config;
        string line;
        model_config.open(classif_config);
//         Classifier * l_classif;
        while (getline(model_config,line))
        {
            string domain=line.substr(0,line.find("\t"));
            string file=line.substr(line.find("\t")+1);
            cerr << domain <<"\t"<< file << endl;
            _list_classifs.push_back(new Classifier(file,domain));
        }
        model_config.close();
        model_config.open(NMT_config);
//         Classifier * l_classif;
        while (getline(model_config,line))
        {
            string domain=line.substr(0,line.find("\t"));
            string file=line.substr(line.find("\t")+1);
            cerr << domain <<"\t"<< file << endl;
            _list_nlu.push_back(new NMT(file,domain));
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
    vector<NMT*> _list_nlu;
    FastText clang;
    FastText cIoT;
    FastText cint;
    FastText cshop;
    qnlu nlu_IOT;
    qnlu nlu_MEDIA;
    cpr::Response *r;
    
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
        response.send(Pistache::Http::Code::Ok, "{\"language_pairs\":[\"en-fr-generic\",\"fr-en-generic\"]}");
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
    bool askNMT(vector<vector<string> > &input, json &output, string &domain, bool debugmode)
    {
        vector<vector<string> > result_batched ;
        auto it_nlu = std::find_if(_list_nlu.begin(), _list_nlu.end(), [&](NMT* l_nlu) 
        {
            return l_nlu->getDomain() == domain;
        }); 
        if (it_nlu != _list_nlu.end())
        {
            (*it_nlu)->batch_NMT(input,result_batched);
        }
        string  translation_concat("");
        string  curr_token("");
        string  word_concat("");
        for (int i=0;i<(int)input.size();i++)
        {
            for (int j=0;j<(int)input.at(i).size();j++)
            {
                
                json j_tmp;
                curr_token=result_batched.at(i).at(j);
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
            string lang=j["language"]; 
            qtokenizer l_tok(lang,false);
            
            
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
            if (debugmode) j.push_back( nlohmann::json::object_t::value_type(string("tokenized_vec"), tokenized_batched ));
            
            if (j.find("domain") != j.end())
            {
                string domain=j["domain"]; 
                string tokenized=j["tokenized"]; 
                istringstream istr(tokenized);

                if (domain.find("iot")==0)
                {
                    
                    std::vector < std::pair < fasttext::real, std::string > > results = askClassification(tokenized,domain,count);
                    j.push_back( nlohmann::json::object_t::value_type(string("intention"), results));
                }
                askNMT(tokenized_batched,j,domain,debugmode);
            }
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
    Lock nluLock;
//     std::vector<Classifier> nluDomains;
    

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main(int argc, char *argv[]) {
    Port port(9009);

    int thr = 8;
    string model_config_classif("model_classif_config.txt");
    string model_config_NMT("model_nlu_config.txt");
    if (argc >= 2) 
    {
        port = std::stol(argv[1]);
        if (argc >= 3)
        {
            thr = std::stol(argv[2]);
            if (argc >= 4)
            {
                model_config_classif = string(argv[3]);
                if (argc >= 5)
                {
                    model_config_NMT = string(argv[4]);
                }
            }
        }
    }

    Address addr(Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;
    

    StatsEndpoint stats(addr,model_config_classif,model_config_NMT);

    stats.init(thr);
    stats.start();
    stats.shutdown();
}
