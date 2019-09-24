// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "rest_server.h"
#include "nmt.h"
#include "utils.h"


void rest_server::init(size_t thr) {
  // Creating the entry point of the REST API.
  Pistache::Port pport(_num_port);
  Address addr(Ipv4::any(), pport);
  httpEndpoint = std::make_shared<Http::Endpoint>(addr);

  auto opts = Http::Endpoint::options().threads(thr).flags(
      Tcp::Options::InstallSignalHandler);
  httpEndpoint->init(opts);
  setupRoutes();
}

void rest_server::start() {
  httpEndpoint->setHandler(router.handler());
  httpEndpoint->serve();
  httpEndpoint->shutdown();
}

void rest_server::setupRoutes() {
  using namespace Rest;

  Routes::Post(router, "/translate/",
               Routes::bind(&rest_server::doTranslationPost, this));

  Routes::Post(router, "/translate_batch/",
              Routes::bind(&rest_server::doTranslationBatchPost, this));

  Routes::Get(router, "/translate/",
              Routes::bind(&rest_server::doTranslationGet, this));  
}


void rest_server::doTranslationGet(const Rest::Request &request,
                                      Http::ResponseWriter response) {
  response.headers().add<Http::Header::AccessControlAllowHeaders>(
      "Content-Type");
  response.headers().add<Http::Header::AccessControlAllowMethods>(
      "GET, POST, DELETE, OPTIONS");
  response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
  response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
  string response_string = "{\"nmt-domains\":[";
  for (int inc = 0; inc < (int)_list_translation_model.size(); inc++) {
    if (inc > 0)
      response_string.append(",");
    response_string.append("\"");
    response_string.append(_list_translation_model.at(inc)->getDomain());
    response_string.append("\"");
  }
  response_string.append("]}");
  if (_debug_mode != 0)
    cerr << "[DEBUG]\t" << currentDateTime() << "\tRESPONSE\t" << response_string << endl;
  response.send(Pistache::Http::Code::Ok, response_string);
}

void rest_server::doTranslationPost(const Rest::Request &request,
                                       Http::ResponseWriter response) {
  response.headers().add<Http::Header::AccessControlAllowHeaders>(
      "Content-Type");
  response.headers().add<Http::Header::AccessControlAllowMethods>(
      "GET, POST, DELETE, OPTIONS");
  response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
  nlohmann::json j = nlohmann::json::parse(request.body());
  
  int count;
  float threshold;
  bool debugmode;
  string domain;
  string lang_src;  
  string lang_tgt;  
  try {
    rest_server::fetchParamWithDefault(j, domain, lang_src, lang_tgt, count, threshold, debugmode);
  } catch (std::runtime_error e) {
    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Bad_Request, e.what());
  }

  if (j.find("text") != j.end()) 
  {
    string text = j["text"];
    string tokenized;
    if (_debug_mode != 0)
      cerr << "[DEBUG]\t" << currentDateTime() << "\t" << "ASK TRANSLATION:\t" << j << endl;
    askTranslation(text, tokenized, j, domain, lang_src, lang_tgt, debugmode);
    j.push_back(nlohmann::json::object_t::value_type(string("tokenized"), tokenized));

    std::string s = j.dump();
    if (_debug_mode != 0)
      cerr << "[DEBUG]\t" << currentDateTime() << "\tRESPONSE\t" << s << endl;
    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok, std::string(s));
  } else {
    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Bad_Request,std::string("The `text` value is required"));
  }
}

void rest_server::doTranslationBatchPost(const Rest::Request &request,
                                       Http::ResponseWriter response) {
  response.headers().add<Http::Header::AccessControlAllowHeaders>(
      "Content-Type");
  response.headers().add<Http::Header::AccessControlAllowMethods>(
      "GET, POST, DELETE, OPTIONS");
  response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
  nlohmann::json j = nlohmann::json::parse(request.body());

  int count;
  float threshold;
  bool debugmode;
  string domain;
  string lang_src;
  string lang_tgt;
  
  try {
    rest_server::fetchParamWithDefault(j, domain, lang_src, lang_tgt, count, threshold, debugmode);
  } catch (std::runtime_error e) {
    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Bad_Request, e.what());
  }
  
  if (j.find("batch_data") != j.end()) {
    for (auto& it: j["batch_data"]){
      if (it.find("text") != it.end()) {
        string text = it["text"];
        string tokenized;
        if (_debug_mode != 0)
          cerr << "[DEBUG]\t" << currentDateTime() << "\tASK TRANSLATION:\t" << it << endl;
        askTranslation(text, tokenized, it, domain, lang_src, lang_tgt, debugmode);
        it.push_back(nlohmann::json::object_t::value_type(string("tokenized"), tokenized));
      }
      else 
      {
        response.headers().add<Http::Header::ContentType>(
            MIME(Application, Json));
        response.send(Http::Code::Bad_Request,
                      std::string("`text` value is required for each item in `batch_data` array"));
      }
    }
    std::string s = j.dump();
    if (_debug_mode != 0)
      cerr << "[DEBUG]\t" << currentDateTime() << "\tRESULT TRANSLATION:\t" << s << endl;
    response.headers().add<Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Http::Code::Ok, std::string(s));
  } else {
    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Bad_Request,
                  std::string("`batch_data` value is required"));
  }
}


void rest_server::fetchParamWithDefault(const nlohmann::json& j, 
                            string& domain, string& lang_src, string& lang_tgt, 
                            int& count,
                            float& threshold,
                            bool& debugmode){
  count = 10;
  threshold = 0.0;
  debugmode = false;

  if (j.find("count") != j.end()) {
    count = j["count"];
  }            
//   string src=j["source"]; 
//             
//   string tgt=j["target"]; 

  if (j.find("source") != j.end()) {
    lang_src = j["source"];
  } else {
    throw std::runtime_error("`source` value is null");
  }
  if (j.find("source") != j.end()) {
    lang_tgt = j["target"];
  } else {
    throw std::runtime_error("`target` value is null");
  }
  if (j.find("threshold") != j.end()) {
    threshold = j["threshold"];
  }
  if (j.find("debug") != j.end()) {
    debugmode = j["debug"];
  }
  if (j.find("domain") != j.end()) {
    domain = j["domain"];
  } else {
    throw std::runtime_error("`domain` value is null");
  }
}

bool rest_server::askTranslation(std::string &text, std::string &tokenized_text, json &output, string &domain, string &lang_src, string &lang_tgt, bool debugmode)
{
    auto it_translation = std::find_if(_list_translation_model.begin(), _list_translation_model.end(), [&](nmt* l_nmt) 
    {
        return (l_nmt->getDomain() == domain && l_nmt->getLangSrc() == lang_src && l_nmt->getLangTgt() == lang_tgt);
    }); 
    if (it_translation == _list_translation_model.end())
    {
        cerr << "[ERROR]\t" << currentDateTime() << "\tRESPONSE\t" << "`domain+lang_src+lang_tgt` values are not valid ("+domain+","+lang_src+"->"+lang_tgt+") for TRANSLATION" << endl;
        return false;
    }
    
    tokenized_text = (*it_translation)->tokenize_str(text);
    std::string tokenized_text_sp = (*it_translation)->spm_segment(tokenized_text);
    std::vector<std::string> tokenized_vec;
    Split(tokenized_text_sp,tokenized_vec," ");
    vector< std::string > tokenized_batched ;
    std::string tokenized_tmp;
    for (int l_inc=0; l_inc < (int)tokenized_vec.size(); l_inc++)
    {
        if (l_inc==0) tokenized_tmp=tokenized_vec[l_inc];
        else tokenized_tmp=tokenized_tmp+" "+tokenized_vec[l_inc];
//         tokenized_tmp.push_back(tokenized_vec[l_inc]);
        if (l_inc == (int)tokenized_vec.size()-1 || ((int)tokenized_vec[l_inc].size() == 1 && (tokenized_vec[l_inc].find("▁.")==0 || tokenized_vec[l_inc].find("▁!")==0  || tokenized_vec[l_inc].find("▁...")==0  || tokenized_vec[l_inc].find("▁?")==0 || tokenized_vec[l_inc].compare("\n")==0)))
        {
            tokenized_batched.push_back(tokenized_tmp);
            tokenized_tmp.clear();
        }
    }
    if (_debug_mode != 0 ) cerr << "LOG: "<< currentDateTime() << "\t" << "BATCH SIZE:\t" << (int)tokenized_batched.size() << endl;
    if ((int)tokenized_tmp.size() > 0)
    {
        tokenized_batched.push_back(tokenized_tmp);
    }    
    return askTranslation(tokenized_batched, output, domain, lang_src, lang_tgt, debugmode);
}

bool rest_server::askTranslation(vector<string> &input, json &output, string &domain, string &lang_src, string &lang_tgt, bool debugmode)
{
    vector<string > translation_output ;
    vector<string > alignement_output_scores ;
    vector<float > translation_scores ;
    auto it_translation = std::find_if(_list_translation_model.begin(), _list_translation_model.end(), [&](nmt* l_nmt) 
    {
        return (l_nmt->getDomain() == domain && l_nmt->getLangSrc() == lang_src && l_nmt->getLangTgt() == lang_tgt);
    }); 
    if (it_translation == _list_translation_model.end())
    {
        cerr << "[ERROR]\t" << currentDateTime() << "\tRESPONSE\t" << "`domain+lang_src+lang_tgt` values are not valid ("+domain+","+lang_src+"->"+lang_tgt+") for TRANSLATION" << endl;
        return false;
    }
    if ((*it_translation)->NMTTranslateBatch(input, translation_output, translation_scores, alignement_output_scores))
    {
        output.push_back(nlohmann::json::object_t::value_type(string("translation"), translation_output));
        output.push_back(nlohmann::json::object_t::value_type(string("translation_scores"), translation_scores));
        if ((int)alignement_output_scores.size() > 0) output.push_back(nlohmann::json::object_t::value_type(string("translation_alignements"), alignement_output_scores));
        return true;
    }
    return false;
}


void rest_server::doAuth(const Rest::Request &request,
                         Http::ResponseWriter response) {
//   printCookies(request);
  response.cookies().add(Http::Cookie("lang", "fr-FR"));
  response.send(Http::Code::Ok);
}

void rest_server::shutdown() {
  httpEndpoint->shutdown(); 
}

// const std::string rest_server::currentDateTime() {
//     time_t     now = time(0);
//     struct tm  tstruct;
//     char       buf[80];
//     tstruct = *localtime(&now);
//     // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
//     // for more information about date/time format
//     strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
// 
//     return buf;
// }


