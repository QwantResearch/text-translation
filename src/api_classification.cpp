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
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <nlohmann/json.hpp>
#include <map>
#include <fasttext/fasttext.h>
#include <sstream>

#include "qtokenizer.h"

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

class StatsEndpoint {
public:
    StatsEndpoint(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    { }

    void init(size_t thr = 2) {
        auto opts = Http::Endpoint::options()
            .threads(thr)
            .flags(Tcp::Options::InstallSignalHandler);
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

    void shutdown() {
        httpEndpoint->shutdown();
    }
    
//     void setLanguage(string lang)
//     {
//         if (tokenizers.find(lang) != tokenizers.end())
//         {
//             qtokenizer* l_qutok= new qtokenizer(lang);
//             
//             pair<string,qtokenizer*> l_pair(lang,l_qutok);
//             tokenizers.insert(l_pair);
//         }
//     }

private:
//     map<string,qtokenizer*> tokenizers;
    FastText clang;
    FastText cint;
    FastText cIoT;
    FastText cpub;
    FastText cshop;
    
    void setupRoutes() {
        using namespace Rest;
        cerr << "Loading models... "<< endl;
        clang.loadModel("../resources/lid.176.ftz");
        cIoT.loadModel("../resources/model_IoT_pre_3.ftz");
        cint.loadModel("../resources/model_query_IA_v11_qua_lowered.ftz");
        cshop.loadModel("../resources/model.pubshop.ftz");
//         cpub.loadModel("../resources/model.pubshop.ftz");
        cerr << "... loading done. "<< endl;
        Routes::Post(router, "/intention/", Routes::bind(&StatsEndpoint::doClassificationPost, this));
        Routes::Get(router, "/intention/", Routes::bind(&StatsEndpoint::doClassificationGet, this));
        Routes::Get(router, "/intentiondomains/", Routes::bind(&StatsEndpoint::doClassificationDomainsGet, this));

    }
    void doClassificationGet(const Rest::Request& request, Http::ResponseWriter response) {
        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Pistache::Http::Code::Ok, "{\"message\":\"success\"}");
    }
    void doClassificationDomainsGet(const Rest::Request& request, Http::ResponseWriter response) {
        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Pistache::Http::Code::Ok, "{\"domains\":[\"language_identification\",\"shopping\",\"advertisement\",\"iot\",\"intention\"]}");
    }
    void doClassificationPost(const Rest::Request& request, Http::ResponseWriter response) 
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
            if (j.find("domain") != j.end())
            {
                string domain=j["domain"]; 
                string tokenized=j["tokenized"]; 
                istringstream istr(tokenized);
                vector<pair<real, string>> results;
    //             auto it_classif = std::find_if(classificationDomains.begin(), classificationDomains.end(), [&](const Classifier& l_classif) {
    //             return l_classif.getDomain() == domain;
    //         }); 
    //             if (it_classif != classificationDomains.end())
    //             {
    //                 results = it_classif->prediction(tokenized,count);
    //             }
                if (domain.find("language_identification")==0)
                {
                    clang.predict(istr,count,results);
                }
                else if (domain.find("shopping")==0)
                {
                    cshop.predict(istr,count,results);
                }
                else if (domain.find("advertisement")==0)
                {
                    cshop.predict(istr,count,results);
                }
                else if (domain.find("iot")==0)
                {
                    cIoT.predict(istr,count,results);
                }
                else if (domain.find("intention")==0)
                {
                    cint.predict(istr,count,results);
                }
                for (int i=0;i<(int)results.size();i++)
                {
                    results.at(i).first=exp(results.at(i).first);
                    results.at(i).second=results.at(i).second.substr(9);
                }
                j.push_back( nlohmann::json::object_t::value_type(string("intention"), results));
            }
            std::string s=j.dump();
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Ok, std::string(s));
        }
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Bad_Request, std::string("Missing things..."));
    }
    void writeLog(string text_to_log)
    {
        
    }

    class Classifier {
    public:
        Classifier(std::string filename, string domain)
        {
            _model.loadModel(filename);
            _domain = domain;
        }
        
        vector<pair<real, string>> prediction(string text, int count)
        {
            vector<pair<real, string>> results;
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
//         fasttext::FastText getModel()
//         {
//             return _model;
//         }

    private:
        std::string _domain;
        fasttext::FastText _model;
    };

    void doAuth(const Rest::Request& request, Http::ResponseWriter response) {
        printCookies(request);
        response.cookies()
            .add(Http::Cookie("lang", "fr-FR"));
        response.send(Http::Code::Ok);
    }

    typedef std::mutex Lock;
    typedef std::lock_guard<Lock> Guard;
    Lock classificationLock;
    std::vector<Classifier> classificationDomains;
    

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main(int argc, char *argv[]) {
    Port port(9009);

    int thr = 8;

    if (argc >= 2) {
        port = std::stol(argv[1]);

        if (argc == 3)
            thr = std::stol(argv[2]);
    }

    Address addr(Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;
    

    StatsEndpoint stats(addr);

    stats.init(thr);
    stats.start();
//     stats.setLanguage("fr");
//     stats.setLanguage("en");
//     stats.setLanguage("de");
//     stats.setLanguage("es");
//     stats.setLanguage("nl");
//     stats.setLanguage("cs");
//     stats.setLanguage("pt");
//     stats.setLanguage("it");
    stats.shutdown();
}
