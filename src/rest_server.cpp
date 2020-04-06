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

  auto opts = Http::Endpoint::options().threads(thr);
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
  
  nlohmann::json response_json;
  
  nlohmann::json languages_json;
  for (int inc = 0; inc < (int)_list_translation_model.size(); inc++) 
  {
    vector<string> lang_src_vec = _list_translation_model.at(inc)->getLangSrcVec();
    vector<string> lang_tgt_vec = _list_translation_model.at(inc)->getLangTgtVec();
    nlohmann::json language_src_json;
    for (int inc_src = 0; inc_src < (int)lang_src_vec.size(); inc_src++) 
    {
        std::vector<std::pair<string,string>> list_pair_tgt_domain_to_add;
        nlohmann::json language_tgt_json;
        for (int inc_tgt = 0; inc_tgt < (int)lang_tgt_vec.size(); inc_tgt++) 
        {
            list_pair_tgt_domain_to_add.push_back(std::pair<string,string>(lang_tgt_vec[inc_tgt], _list_translation_model.at(inc)->getDomain()));
        }
        auto languages_json_iter_src = languages_json.find(lang_src_vec[inc_src]);
        if (languages_json_iter_src ==  languages_json.end())
        {
            auto it = list_pair_tgt_domain_to_add.begin();
            while (it != list_pair_tgt_domain_to_add.end())
            {
                nlohmann::json language_tgt_json;
                std::vector<std::string> tmp_domains={it->second};
                language_tgt_json.push_back(nlohmann::json::object_t::value_type(it->first, tmp_domains));
                languages_json.push_back(nlohmann::json::object_t::value_type(string(lang_src_vec[inc_src]), language_tgt_json));
                it++;
            }
        }
        else
        {
            auto it = list_pair_tgt_domain_to_add.begin();
            while (it != list_pair_tgt_domain_to_add.end())
            {
                auto languages_json_iter_tgt = languages_json_iter_src->find(it->first);
                if (languages_json_iter_tgt == languages_json_iter_src->end())
                {
                    std::vector<std::string> tmp_domains={it->second};
                    languages_json_iter_src->push_back(nlohmann::json::object_t::value_type(it->first, tmp_domains));
                }
                else
                {
                    languages_json_iter_tgt->push_back(it->second);
                }
                it++;
            }
        }
    }
  }
  response_json.push_back(nlohmann::json::object_t::value_type(string("languages"), languages_json));
  string response_string = response_json.dump();
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
  int debugmode;
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
  int debugmode;
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


void rest_server::fetchParamWithDefault(const nlohmann::json& j, std::__cxx11::string& domain, std::__cxx11::string& lang_src, std::__cxx11::string& lang_tgt, int& count, float& threshold, int& debugmode)
{
  count = 10;
  threshold = 0.0;
  debugmode = _debug_mode;

  if (j.find("count") != j.end()) {
    count = j["count"];
  }            
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
  if (_debug_mode != 0) cerr << "[DEBUG]\t" << currentDateTime() << "\tdomain:\t" << domain << "\tsource:\t" << lang_src << "\ttarget:\t" << lang_tgt << "\tthreshold:\t" << threshold << "\tdebug:\t" << debugmode << "\tcount:\t" << count  << endl;
}

bool rest_server::askTranslation(std::string &text, std::string &tokenized_text, json &output, string &domain, string &lang_src, string &lang_tgt, int debugmode)
{
    auto it_translation = std::find_if(_list_translation_model.begin(), _list_translation_model.end(), [&](nmt* l_nmt) 
    {
        return (l_nmt->getDomain() == domain && l_nmt->isSrcLang(lang_src) && l_nmt->isTgtLang(lang_tgt));
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
        if ((int)tokenized_tmp.size() == 0) tokenized_tmp=tokenized_vec[l_inc];
        else tokenized_tmp=tokenized_tmp+" "+tokenized_vec[l_inc];
//         tokenized_tmp.push_back(tokenized_vec[l_inc]);
        if (l_inc == (int)tokenized_vec.size()-1 || tokenized_vec[l_inc].compare("▁.")==0 || tokenized_vec[l_inc].compare("▁!")==0  || tokenized_vec[l_inc].compare("▁...")==0  || tokenized_vec[l_inc].compare("▁?")==0 || tokenized_vec[l_inc].compare(".")==0 || tokenized_vec[l_inc].compare("!")==0  || tokenized_vec[l_inc].compare("...")==0  || tokenized_vec[l_inc].compare("?")==0 || tokenized_vec[l_inc].compare("\n")==0)
        {
            tokenized_batched.push_back(tokenized_tmp);
            tokenized_tmp.clear();
        }
    }
    if ((int)tokenized_tmp.size() > 0)
    {
        tokenized_batched.push_back(tokenized_tmp);
    }    
    if (_debug_mode != 0 ) 
    {
        cerr << "[DEBUG]\t"<< currentDateTime() << "\t" << "BATCH SIZE:\t" << (int)tokenized_batched.size() << endl;
        for (int l_inc=0; l_inc < (int)tokenized_batched.size(); l_inc++)
        {
            cerr << "[DEBUG]\t"<< currentDateTime() << "\t" << "BATCH CONTENT:\t" << l_inc << "\t"<< tokenized_batched[l_inc] << endl;
        }
    }
    return askTranslation(tokenized_batched, output, domain, lang_src, lang_tgt, debugmode);
}

bool rest_server::askTranslation(vector<string> &input, json &output, string &domain, string &lang_src, string &lang_tgt, int debugmode)
{
    vector<string > translation_output ;
    vector<string > translation_raw_output ;
    vector<string > alignement_output_scores ;
    vector<float > translation_scores ;
    auto it_translation = std::find_if(_list_translation_model.begin(), _list_translation_model.end(), [&](nmt* l_nmt) 
    {
        return (l_nmt->getDomain() == domain && l_nmt->isSrcLang(lang_src) && l_nmt->isTgtLang(lang_tgt));
    }); 
    if (it_translation == _list_translation_model.end())
    {
        cerr << "[ERROR]\t" << currentDateTime() << "\tRESPONSE\t" << "`domain+lang_src+lang_tgt` values are not valid ("+domain+","+lang_src+"->"+lang_tgt+") for TRANSLATION" << endl;
        return false;
    }
    if ((*it_translation)->NMTTranslateBatch(input, translation_output, translation_raw_output, translation_scores, alignement_output_scores))
    {
        output.push_back(nlohmann::json::object_t::value_type(string("translation"), translation_output));
        output.push_back(nlohmann::json::object_t::value_type(string("translation_scores"), translation_scores));
        if ((int)alignement_output_scores.size() > 0) output.push_back(nlohmann::json::object_t::value_type(string("raw_translation"), translation_raw_output));
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


