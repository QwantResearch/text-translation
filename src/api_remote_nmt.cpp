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
#include "spm.h"
#include <qsmt.h>

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

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}


// class Classifier {
// public:
//     Classifier(std::string filename, string domain)
//     {
//         _model.loadModel(filename.c_str());
//         _domain = domain;
//     }
//     
//     vector < pair < fasttext::real, string > > prediction(string text, int count)
//     {
//         vector <pair <fasttext::real, string> > results;
//         istringstream istr(text);
//         _model.predict(istr,count,results);
//         for (int i=0;i<(int)results.size();i++)
//         {
//             results.at(i).first=exp(results.at(i).first);
//             results.at(i).second=results.at(i).second.substr(9);
//         }
//         return results;
//     }
//     std::string getDomain()
//     {
//         return _domain;
//     }
// 
// private:
//     std::string _domain;
//     fasttext::FastText _model;
// };
  
class NMT {
public:
    std::string _domain;
    std::string _src;
    std::string _tgt;
    int _local;

    NMT(std::string dirname, string domain, string src, string tgt, string bpe, int local)
    {
        cerr << "*************************************" <<endl;
        cerr << "Model name: |" << dirname <<"|"<<endl;
	cerr << "Model domain: |" << domain <<"|"<<endl;
        cerr << "Model details: |"<< string(src+"-"+tgt+"-"+domain)<<"|"<<endl;
        if (local == 1)
        {
                _model.LoadModel(dirname);
                cerr << "Model is locally loaded" <<endl;
        }
        else if (local == 2)
        {
                _model.LoadGraph(dirname);
                cerr << "Graph is locally loaded" <<endl;
        }
        else if (local == 3)
        {
                std::string lhost=dirname.substr(0,dirname.find("/"));
                std::string lroute=dirname.substr((int)dirname.find("/")+1,(int)dirname.size()-dirname.find("/"));
                std::string lport=lhost.substr((int)lhost.find(":")+1,((int)lhost.size())-lhost.find(":"));
                lhost=lhost.substr(0,lhost.find(":"));
                cerr << "Host: |" << lhost << "|" << endl;
                cerr << "Port: |" << lport << "|" << endl;
                cerr << "Route: |" << lroute << "|" << endl;
                cerr << "Domain: |" << dirname << "|" <<endl;
                
                _modelsmt.setqsmt(lhost,lport, lroute, dirname);
                cerr << "SMT is set" <<endl;
        }
        else _model.LoadModel(string(src+"-"+tgt+"-"+domain), grpc::CreateChannel(dirname,grpc::InsecureChannelCredentials()));
        cerr << "*************************************" <<endl;
        _domain = domain;
        _spm_model = false;
        if ((int)bpe.find("spm") > -1)
        {
            _spm= new spm(bpe);
            _spm_model = true;
        }
        else
        {
            _bpe = new BPE(bpe);
        }
        _src=src;
        _tgt=tgt;
        _local=local;

    }
    
    bool batch_NMT(vector<vector<string>>& input, vector<vector<string>>& output, vector<float>& scores_result_batched)
    {
        if (_local == 1) return _model.NMTBatch(input,output,scores_result_batched);
        if (_local == 2) return _model.NMTBatchGraph(input,output,scores_result_batched);
        if (_local == 3) return _modelsmt.NMTBatchSMT(input,output,scores_result_batched);
        return _model.NMTBatchOnline(input,output,scores_result_batched);
    }
    bool batch_NMT(string& input, vector<vector<string>>& output, vector<float>& scores_result_batched)
    {
        vector<string> to_process ;
        if (_local != 3)
        {
            if (_spm_model) to_process=_spm->segment(input);
            else to_process = _bpe->Segment(input);
        }
        else
        {
            Split(input,to_process," ");
        }
// 	cerr << input << endl;
        vector<string> to_process_tmp;
        vector<vector<string> > to_translate;

	for (int i=0; (int)i<(int)to_process.size() ; i++)
	{
                to_process_tmp.push_back(to_process[i]);
//   		cerr << to_process[i] << endl;
		if (((i > 0) && (i<(int)to_process.size() - 1) && (int)to_process[i].find(".") == 0 ) || i == (int)to_process.size()-1)
                {
                       
                       to_translate.push_back(to_process_tmp);
                       to_process_tmp.clear();
                }
// 		cerr << to_process.at(i)<<endl;
	}
	if ((int)to_process_tmp.size() > 0)
        {
            to_translate.push_back(to_process_tmp);
            to_process_tmp.clear();
        }
       vector<vector<string> > to_translate_sentence;
       vector<vector<string> > translation_result;
       vector<float> scores_result;

//        to_translate.push_back(to_process);
        for (int i=0; (int)i<(int)to_translate.size() ; i++)
        {
            to_translate_sentence.clear();
            translation_result.clear();
            scores_result.clear();
            to_translate_sentence.push_back(to_translate.at(i));
            switch(_local)
            {
              case 1:
                if (! _model.NMTBatch(to_translate_sentence,translation_result,scores_result)) return false;
                break;
              case 3:
                if (! _modelsmt.NMTBatchSMT(to_translate_sentence,translation_result,scores_result)) return false;
                break;
              default:
                if (! _model.NMTBatchOnline(to_translate_sentence,translation_result,scores_result)) return false;
                break;
            }
            
            
            for (int j=0; j<(int)translation_result.size(); j++)
            {
                if (i == 0) 
                {
                    output.push_back(translation_result.at(j));
                    scores_result_batched.push_back(scores_result.at(i));
                }
                else
                {
                    for (int k=0; k<(int)translation_result.at(j).size(); j++)
                    {
                        output.at(j).push_back(translation_result.at(j).at(k));
                    }
                    scores_result_batched.at(j)=scores_result_batched.at(j)+scores_result.at(j);
                }
            }
            
//             if (_local == 1) return _model.NMTBatch(to_translate,output,scores_result_batched);
//             if (_local == 3) return _modelsmt.NMTBatchSMT(to_translate,output,scores_result_batched);
//             return _model.NMTBatchOnline(to_translate,output,scores_result_batched);
            
        }
        return true;
//         if (_local == 1) return _model.NMTBatch(to_translate,output,scores_result_batched);
//         if (_local == 3) return _modelsmt.NMTBatchSMT(to_translate,output,scores_result_batched);
//         return _model.NMTBatchOnline(to_translate,output,scores_result_batched);
    }
    std::string getDomain()
    {
        return _domain;
    }


private:
    qnmt _model;
    qsmt _modelsmt;
    BPE* _bpe;
    spm* _spm;
    bool _spm_model;
};


class StatsEndpoint {
public:
    StatsEndpoint(Address addr, string NMT_config, int debugmode)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    { 
        ifstream model_config;
        string line;
        _debugmode=debugmode;
        model_config.open(NMT_config);
        while (getline(model_config,line))
        {
            vector<string> vec_line;
            if ((int)line.find("#") != 0)
            {
            Split(line,vec_line,"\t");
            string src=vec_line.at(0);
            string tgt=vec_line.at(1);
            string bpe=vec_line.at(2);
            string file=vec_line.at(3);
            string domain=vec_line.at(4);
            int local=atoi(vec_line.at(5).c_str());
            _list_nmt.push_back(new NMT(file,domain,src,tgt,bpe,local));
            }
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
//     vector<Classifier*> _list_classifs;
    vector<NMT*> _list_nmt;
    int _debugmode;
    
    void setupRoutes() {
        using namespace Rest;
        Routes::Post(router, "/translate/", Routes::bind(&StatsEndpoint::doNMTPost, this));
        Routes::Get(router, "/translate/", Routes::bind(&StatsEndpoint::doNMTLanguagesGet, this));
        Routes::Options(router, "/translate/", Routes::bind(&StatsEndpoint::doNMTOptions, this));
//         Routes::Get(router, "/languages/", Routes::bind(&StatsEndpoint::doNMTLanguagesGet, this));

    }
    void doNMTOptions(const Rest::Request& request, Http::ResponseWriter response) {
        response.headers().add<Http::Header::AccessControlAllowHeaders>("Content-Type");
        response.headers().add<Http::Header::AccessControlAllowMethods>("GET, POST, DELETE, OPTIONS");
        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Pistache::Http::Code::No_Content, "{\"message\":\"success\"}");
    }

    void doNMTLanguagesGet(const Rest::Request& request, Http::ResponseWriter response) {
        response.headers().add<Http::Header::AccessControlAllowHeaders>("Content-Type");
        response.headers().add<Http::Header::AccessControlAllowMethods>("GET, POST, DELETE, OPTIONS");
        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        string response_str="{\"models\":[";
        for (int i=0; i<(int) _list_nmt.size(); i++)
        {
            if ( i != 0 ) response_str.append(",");
            response_str.append("\"");
            response_str.append(_list_nmt.at(i)->_src);
            response_str.append("-");
            response_str.append(_list_nmt.at(i)->_tgt);
            response_str.append("-");
            response_str.append(_list_nmt.at(i)->_domain);
            response_str.append("\"");
        }
        response_str.append("]}");
        if (_debugmode > 0)
        {
            cerr << "LOG: "<< currentDateTime() << "\t" << response_str << endl;
        }
        response.send(Pistache::Http::Code::Ok, response_str);
    }
    
//     std::vector < std::pair < fasttext::real, std::string > > askClassification(std::string text, std::string domain, int count)
//     {
//         std::vector < std::pair < fasttext::real, std::string > > to_return;
//         istringstream istr(text);
//         auto it_classif = std::find_if(_list_classifs.begin(), _list_classifs.end(), [&](Classifier* l_classif) 
//         {
//             return l_classif->getDomain() == domain;
//         }); 
//         if (it_classif != _list_classifs.end())
//         {
//             to_return = (*it_classif)->prediction(text,count);
//         }
//         return to_return;
//     }
//     bool askNMT(vector<vector<string> > &input, json &output, string &domain, string &src, string &tgt, bool debugmode)
//     {
//         vector<vector<string> > result_batched ;
//         
//         auto it_nmt = std::find_if(_list_nmt.begin(), _list_nmt.end(), [&](NMT* l_nmt) 
//         {
//             return (l_nmt->_domain == domain && l_nmt->_src == src && l_nmt->_tgt == tgt) ;
//         }); 
//         if (it_nmt != _list_nmt.end())
//         {
//             (*it_nmt)->batch_NMT(input,result_batched);
//         }
//         string  translation_concat("");
//         string  curr_token("");
//         string  word_concat("");
//         for (int i=0;i<(int)result_batched.size();i++)
//         {
//             for (int j=0;j<(int)result_batched.at(i).size();j++)
//             {
//                 json j_tmp;
//                 curr_token=result_batched.at(i).at(j);
//                 cerr << "output: " << curr_token << endl;
//                 if (translation_concat.length() > 0) translation_concat.append(" ");
//                 translation_concat.append(curr_token);
//             }
//         }
//         int sep_pos = (int)translation_concat.find("@@");
//         while (sep_pos > -1)
//         {
//             translation_concat=translation_concat.erase(sep_pos,3);
//             sep_pos = (int)translation_concat.find("@@");
//             
//         }
//         output.push_back(nlohmann::json::object_t::value_type(string("translation"), translation_concat));
//         
//     }
    
    bool askNMT(string &input, json &output, string &domain, string &src, string &tgt, bool debugmode)
    {
        vector<vector<string> > result_batched ;
        vector<float> scores_result_batched ;
        vector<string> translation_concat_vec;
        vector<float> translation_scores_vec;
        string  translation_concat("");
        string  curr_token("");
        string  word_concat("");

        if ((int)input.size() > 1 )
	{
        	auto it_nmt = std::find_if(_list_nmt.begin(), _list_nmt.end(), [&](NMT* l_nmt) 
	        {
	            return (l_nmt->_domain == domain && l_nmt->_src == src && l_nmt->_tgt == tgt) ;
	        }); 
	        if (it_nmt != _list_nmt.end())
	        {
	            (*it_nmt)->batch_NMT(input,result_batched,scores_result_batched);
	        }
	        else
	        {
	            return false;
	        }
	    	for (int i=0;i<(int)result_batched.size();i++)
	       	{
       	     		int j=0;
       	     		for (j=0;j<(int)result_batched.at(i).size();j++)
       	     		{
       	         		json j_tmp;
       	         		curr_token=result_batched.at(i).at(j);
//     	            cerr << "output: " << curr_token << endl;
       	         		if (translation_concat.length() > 0) translation_concat.append(" ");
       	         		translation_concat.append(curr_token);
       	     		}
       	     		int sep_pos = (int)translation_concat.find("@@");
       	     		while (sep_pos > -1)
       	     		{
       	         		translation_concat=translation_concat.erase(sep_pos,3);
       	         		sep_pos = (int)translation_concat.find("@@");
       	     		}
	            	translation_concat_vec.push_back(translation_concat);
//      	       translation_scores_vec.push_back(atof(result_batched.at(i).at(j).c_str()));
	       	     	translation_concat.clear();
	        }
	}
	else
	{
		translation_concat_vec.push_back(input);
		scores_result_batched.push_back(-1000.00);
	}
        output.push_back(nlohmann::json::object_t::value_type(string("translation"), translation_concat_vec));
        output.push_back(nlohmann::json::object_t::value_type(string("translation_scores"), scores_result_batched));
        return true;
    }
//     void doNMTGet(const Rest::Request& request, Http::ResponseWriter response) {
//         response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
//         response.send(Pistache::Http::Code::Ok, "{\"message\":\"success\"}");
//     }
    void doNMTPost(const Rest::Request& request, Http::ResponseWriter response) 
    {
        response.headers().add<Http::Header::AccessControlAllowHeaders>("Content-Type");
        response.headers().add<Http::Header::AccessControlAllowMethods>("GET, POST, DELETE, OPTIONS");
        response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        nlohmann::json j = nlohmann::json::parse(request.body());
        int count=10;
        bool debugmode = false;
        if (_debugmode > 0)
        {
            cerr << "LOG [ASK]: "<< currentDateTime() << "\t" << request.body() << endl;
        }
        
        if (j.find("source") == j.end())
        {
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Bad_Request, "{\"Error\":\"parameter source is needed\"}");
        }

        if (j.find("target") == j.end())
        {
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Bad_Request, "{\"Error\":\"parameter target is needed\"}");
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
            string src=j["source"]; 
            string tgt=j["target"]; 
            qtokenizer l_tok(src,false);
            
            
            j.push_back( nlohmann::json::object_t::value_type(string("tokenized"), l_tok.tokenize_str(text) ));
/*            vector<string> tokenized_vec = l_tok.tokenize(text) ;
            vector<vector<string> > tokenized_batched ;
            vector<string> line_tokenized;
            for (int l_inc=0; l_inc < (int)tokenized_vec.size(); l_inc++)
            {
                line_tokenized.push_back(tokenized_vec[l_inc]);
		cerr << tokenized_vec[l_inc].compare(".") << endl;
		cerr << tokenized_vec[l_inc].compare("\n") << endl;
                if (l_inc == (int)tokenized_vec.size()-1 || ((int)tokenized_vec[l_inc].size() > 0 && (tokenized_vec[l_inc].compare(".")==0 || tokenized_vec[l_inc].compare("\n")==0)))
                {
                    tokenized_batched.push_back(line_tokenized);
                    line_tokenized.clear();
                }
            }
//             cerr << tokenized_batched.size() << endl;
            if (debugmode) j.push_back( nlohmann::json::object_t::value_type(string("tokenized_vec"), tokenized_batched )); */
            if (j.find("domain") != j.end())
            {
                string domain=j["domain"]; 
                string tokenized=j["tokenized"]; 
                if (! askNMT(tokenized,j,domain,src,tgt,debugmode))
                {
                    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
                    response.send(Http::Code::Bad_Request, "{\"Error\":\"combination src, tgt, domain not available!\"}");
                }
            }
        }
        else
        {
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Bad_Request, "{\"Error\":\"parameter text is needed\"}");
        }
        std::string s=j.dump();
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        if (_debugmode > 0)
        {
            cerr << "LOG [RES]: "<< currentDateTime() << "\t" << s << endl;
        }
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

    string model_config_NMT("model_nmt_config.txt");
    int debugmode=0; //0 = no log ; 1 = info ; 2 = full logs
    if (argc >= 2) 
    {
        port = std::stol(argv[1]);
        if (argc >= 3)
        {
            thr = std::stol(argv[2]);
            if (argc >= 4)
            {
                model_config_NMT = string(argv[3]);
                if (argc >= 5)
                {
                    debugmode = std::atoi(argv[4]);
                }
            }
        }
    }

    Address addr(Ipv4::any(), port);

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;
    

    StatsEndpoint stats(addr,model_config_NMT,debugmode);

    stats.init(thr);
    stats.start();
    stats.shutdown();
}
