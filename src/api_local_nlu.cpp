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
// #include <curlpp/cURLpp.hpp>
// #include <curlpp/Easy.hpp>
// #include <curlpp/Options.hpp>
// #include <curlpp/Exception.hpp>
// #include <curlpp/Infos.hpp>
#include "cpr/cpr.h"
#include "qtokenizer.h"
#include "qnlu.h"

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
//         fasttext::FastText getModel()
//         {
//             return _model;
//         }

private:
    std::string _domain;
    fasttext::FastText _model;
};

class NLU {
public:
    NLU(std::string dirname, string domain)
    {
        _model.LoadModel(dirname);
        _domain = domain;
    }
    
    bool batch_NLU(vector<vector<string>>& input, vector<vector<string>>& output)
    {
        return _model.NLUBatch(input,output);
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
    qnlu _model;
};


class StatsEndpoint {
public:
    StatsEndpoint(Address addr, string classif_config, string NLU_config)
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
        model_config.open(NLU_config);
//         Classifier * l_classif;
        while (getline(model_config,line))
        {
            string domain=line.substr(0,line.find("\t"));
            string file=line.substr(line.find("\t")+1);
            cerr << domain <<"\t"<< file << endl;
            _list_nlu.push_back(new NLU(file,domain));
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
    vector<Classifier*> _list_classifs;
    vector<NLU*> _list_nlu;
    FastText clang;
    FastText cIoT;
    FastText cint;
    FastText cshop;
    qnlu nlu_IOT;
    qnlu nlu_MEDIA;
    cpr::Response *r;
    
    void setupRoutes() {
        using namespace Rest;
        cerr << "Loading models... "<< endl;
//         clang.loadModel("../resources/lid.176.ftz");
//         cIoT.loadModel("../resources/model_IoT_pre_3.ftz");
//         cint.loadModel("../resources/model_query_IA_v11_qua_lowered.ftz");
//         cshop.loadModel("../resources/model.pubshop.ftz");
//         nlu_IOT.LoadModel("../resources/NLU/IoT/1528899408/");
//         nlu_MEDIA.LoadModel("../resources/NLU/MEDIA/1525818599/");
//         cpub.loadModel("../resources/model.pubshop.ftz");
        cerr << "... loading done. "<< endl;
        Routes::Post(router, "/intention/", Routes::bind(&StatsEndpoint::doClassificationPost, this));
        Routes::Get(router, "/intention/", Routes::bind(&StatsEndpoint::doClassificationGet, this));
        Routes::Post(router, "/nlu/", Routes::bind(&StatsEndpoint::doNLUPost, this));
        Routes::Get(router, "/nlu/", Routes::bind(&StatsEndpoint::doNLUGet, this));
        Routes::Get(router, "/nludomains/", Routes::bind(&StatsEndpoint::doNLUDomainsGet, this));
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
    void doNLUDomainsGet(const Rest::Request& request, Http::ResponseWriter response) {
        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Pistache::Http::Code::Ok, "{\"domains\":[\"media\",\"iot\"]}");
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
                std::vector < std::pair < fasttext::real, std::string > > results;
                results = askClassification(tokenized,domain,count);
                j.push_back( nlohmann::json::object_t::value_type(string("intention"), results));
            }
            std::string s=j.dump();
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Ok, std::string(s));
        }
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
//         response.headers().add<Http::Header::ContentType>(MIME('Powered-By', 'Qwant Research'));
        response.send(Http::Code::Bad_Request, std::string("Missing things..."));
    }    
    
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
    vector<float> ask_Qmaps(string& page, std::string& output)
    {
      
      r= new cpr::Response(cpr::Get(cpr::Url{"http://localhost:4000/autocomplete"}, cpr::Parameters{{"q", "Paris"}}));
//       cerr << r.status_code << endl;                  // 200
//       cerr << r.header["content-type"] << endl;       // application/json; charset=utf-8
//       cerr << r.text << endl;                         // JSON text string
      cerr << "TEST" << endl;
//       delete(r);
    }
    bool process_localization(string& input, json &output)
    {
        string token(input.c_str());
        if (input.find("à ") == 0) token=input.substr(3);
        if (input.find("au dessus de ") == 0) token=input.substr(13);
        if (input.find("au ") == 0) token=input.substr(4);
        if (input.find("vers ") == 0) token=input.substr(5);
        output.push_back(nlohmann::json::object_t::value_type(string("lat"), 0.0 ));
        output.push_back(nlohmann::json::object_t::value_type(string("long"), 0.0 ));
        output.push_back(nlohmann::json::object_t::value_type(string("label"), token ));
//         ask_Qmaps(token,token);
    }
    bool askNLU(vector<vector<string> > &input, json &output, string &domain, bool debugmode)
    {
        vector<vector<string> > result_batched ;
        auto it_nlu = std::find_if(_list_nlu.begin(), _list_nlu.end(), [&](NLU* l_nlu) 
        {
            return l_nlu->getDomain() == domain;
        }); 
        if (it_nlu != _list_nlu.end())
        {
            (*it_nlu)->batch_NLU(input,result_batched);
        }
        
        json i_tmp = json::array();
        json k_tmp = json::array();
        json t_tmp;
        string  prev_tag("");
        string  curr_tag("");
        string  word_concat("");
        for (int i=0;i<(int)input.size();i++)
        {
            for (int j=0;j<(int)input.at(i).size();j++)
            {
                json j_tmp;
                curr_tag=result_batched.at(i).at(j);
                if (curr_tag.find("I-") == 0)
                {
                    curr_tag=curr_tag.substr(2);
                }
                if (curr_tag.find("B-") == 0)
                {
                    curr_tag=curr_tag.substr(2);
                }
                j_tmp.push_back(nlohmann::json::object_t::value_type(string("word"), input.at(i).at(j) ));
                j_tmp.push_back(nlohmann::json::object_t::value_type(string("tag"), result_batched.at(i).at(j) ));
                j_tmp.push_back(nlohmann::json::object_t::value_type(string("value"), string("")));
                i_tmp.push_back(j_tmp);
                if ((int)prev_tag.length() > 0 && prev_tag.compare(curr_tag) != 0)
                {
                    t_tmp.push_back(nlohmann::json::object_t::value_type(string("phrase"), word_concat ));
                    t_tmp.push_back(nlohmann::json::object_t::value_type(string("tag"), prev_tag ));
                    json j_value({});
                    if (prev_tag.find("loc_city") == 0)
                    {
                        process_localization(word_concat,j_value);
                    }
                    t_tmp.push_back(nlohmann::json::object_t::value_type(string("value"), j_value));
                    word_concat=input.at(i).at(j);
                    k_tmp.push_back(t_tmp);
                    t_tmp.clear();
                }
                else
                {
                    if (word_concat.length() > 0) word_concat.append(" ");
                    word_concat.append(input.at(i).at(j));
                }
                prev_tag=curr_tag;
            }
        }
        if  ((int)word_concat.length() > 0)
        {
            t_tmp.push_back(nlohmann::json::object_t::value_type(string("phrase"), word_concat ));
            t_tmp.push_back(nlohmann::json::object_t::value_type(string("tag"), prev_tag ));
            json j_value({});
            if (prev_tag.find("loc_city") == 0)
            {
                process_localization(word_concat,j_value);
            }
            t_tmp.push_back(nlohmann::json::object_t::value_type(string("value"), j_value));
            k_tmp.push_back(t_tmp);
        }
        if (debugmode) output.push_back(nlohmann::json::object_t::value_type(string("NLU_DEBUG"), i_tmp));
        output.push_back(nlohmann::json::object_t::value_type(string("NLU"), k_tmp));
        
    }
    
    void doNLUGet(const Rest::Request& request, Http::ResponseWriter response) {
        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        response.send(Pistache::Http::Code::Ok, "{\"message\":\"success\"}");
    }
    void doNLUPost(const Rest::Request& request, Http::ResponseWriter response) 
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
//             vector<vector<string> > result_batched ;
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
                    
//                     nlu_IOT.NLUBatch(tokenized_batched,result_batched);
                    std::vector < std::pair < fasttext::real, std::string > > results = askClassification(tokenized,domain,count);
                    j.push_back( nlohmann::json::object_t::value_type(string("intention"), results));
                }
//                 else if (domain.find("media")==0)
//                 {
//                     askNLU(tokenized_batched,j,domain);
//                     nlu_MEDIA.NLUBatch(tokenized_batched,result_batched);
//                 }
                askNLU(tokenized_batched,j,domain,debugmode);
//                 json i_tmp = json::array();
//                 json k_tmp = json::array();
//                 json t_tmp;
//                 string  prev_tag("");
//                 string  curr_tag("");
//                 string  word_concat("");
//                 for (int i=0;i<(int)tokenized_batched.size();i++)
//                 {
//                     for (int j=0;j<(int)tokenized_batched.at(i).size();j++)
//                     {
//                         json j_tmp;
//                         curr_tag=result_batched.at(i).at(j);
//                         if (curr_tag.find("I-") == 0)
//                         {
//                             curr_tag=curr_tag.substr(2);
//                         }
//                         if (curr_tag.find("B-") == 0)
//                         {
//                             curr_tag=curr_tag.substr(2);
//                         }
//                         j_tmp.push_back(nlohmann::json::object_t::value_type(string("word"), tokenized_batched.at(i).at(j) ));
//                         j_tmp.push_back(nlohmann::json::object_t::value_type(string("tag"), result_batched.at(i).at(j) ));
//                         j_tmp.push_back(nlohmann::json::object_t::value_type(string("value"), string("")));
//                         i_tmp.push_back(j_tmp);
//                         if ((int)prev_tag.length() > 0 && prev_tag.compare(curr_tag) != 0)
//                         {
//                             t_tmp.push_back(nlohmann::json::object_t::value_type(string("phrase"), word_concat ));
//                             t_tmp.push_back(nlohmann::json::object_t::value_type(string("tag"), prev_tag ));
//                             t_tmp.push_back(nlohmann::json::object_t::value_type(string("value"), string("")));
//                             word_concat=tokenized_batched.at(i).at(j);
//                             k_tmp.push_back(t_tmp);
//                             t_tmp.clear();
//                         }
//                         else
//                         {
//                             if (word_concat.length() > 0) word_concat.append(" ");
//                             word_concat.append(tokenized_batched.at(i).at(j));
//                         }
//                         prev_tag=curr_tag;
//                     }
//                 }
//                 if  ((int)word_concat.length() > 0)
//                 {
//                     t_tmp.push_back(nlohmann::json::object_t::value_type(string("phrase"), word_concat ));
//                     t_tmp.push_back(nlohmann::json::object_t::value_type(string("tag"), prev_tag ));
//                     t_tmp.push_back(nlohmann::json::object_t::value_type(string("value"), string("")));
//                     k_tmp.push_back(t_tmp);
//                 }
//                 if (debugmode) j.push_back(nlohmann::json::object_t::value_type(string("NLU_DEBUG"), i_tmp));
//                 j.push_back(nlohmann::json::object_t::value_type(string("NLU"), k_tmp));
//                 j.push_back( nlohmann::json::object_t::value_type(string("intention"), results));
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
    string model_config_NLU("model_nlu_config.txt");
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
                    model_config_NLU = string(argv[4]);
                }
            }
        }
    }

    Address addr(Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;
    

    StatsEndpoint stats(addr,model_config_classif,model_config_NLU);

    stats.init(thr);
    stats.start();
    stats.shutdown();
}
