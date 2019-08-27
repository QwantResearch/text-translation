// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "rest_server.h"
#include "nmt.h"
#include "utils.h"

rest_server::rest_server(std::string &config_file, int &threads, int debug) 
{
    std::string line;
    int port=9009;
    YAML::Node config;

    try 
    {
    // Reading the configuration file for filling the options.
        config = YAML::LoadFile(config_file);
        cout << "[INFO]\tDomain\t\tLocation/filename\t\tlanguage"<< endl;
        threads = config["threads"].as<int>() ;
        port =  config["port"].as<int>() ;
        debug =  config["debug"].as<int>() ;
        YAML::Node modelconfig = config["models"]; 
        for (const auto& modelnode: modelconfig)
        {
            std::string domain=modelnode.first.as<std::string>();
            YAML::Node modelinfos = modelnode.second;
            std::string nmt_model_filename=modelinfos["nmt_model"].as<std::string>();
            std::string lang=modelinfos["language"].as<std::string>();
            try 
            {
//                 shared_ptr<Channel> channel = CreateChannel(nmt_model_filename,grpc::InsecureChannelCredentials());
                nmt* nmt_pointer = new nmt(nmt_model_filename, domain, lang);
                _list_translation_model.push_back(nmt_pointer);
                cout << "\t===> loaded" << endl;
            } 
            catch (invalid_argument& inarg) 
            {
                cerr << "[ERROR]\t" << inarg.what() << endl;
                continue;
            }
        }
    } catch (YAML::BadFile& bf) {
      cerr << "[ERROR]\t" << bf.what() << endl;
      exit(1);
    }
    _nbr_threads=threads;
    cout << "[INFO]\tnumber of threads:\t"<< _nbr_threads << endl;
    cout << "[INFO]\tport used:\t"<< port << endl;
    if (debug > 0) cout << "[INFO]\tDebug mode activated" << endl;
    else cout << "[INFO]\tDebug mode desactivated" << endl;
    if ((int)_list_translation_model.size() == 0) 
    {
        cerr << "[ERROR]\tNo nmt model loaded, exiting." << endl;
        exit(1);
    }
    
    
    
    // Creating the entry point of the REST API.
    Pistache::Port pport(port);
    Address addr(Ipv4::any(), pport);
    httpEndpoint = std::make_shared<Http::Endpoint>(addr);
    _debug_mode = debug;

}


void rest_server::init() {
  auto opts = Http::Endpoint::options().threads(_nbr_threads).flags(
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
  string lang;  
  try {
    rest_server::fetchParamWithDefault(j, domain, lang, count, threshold, debugmode);
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
    askTranslate(text, tokenized, j, domain, lang, debugmode);
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
  string lang;
  
  try {
    rest_server::fetchParamWithDefault(j, domain, lang, count, threshold, debugmode);
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
        askTranslate(text, tokenized, it, domain, lang, debugmode);
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
                            string& domain, string& lang, 
                            int& count,
                            float& threshold,
                            bool& debugmode){
  count = 10;
  threshold = 0.0;
  debugmode = false;

  if (j.find("count") != j.end()) {
    count = j["count"];
  }
  if (j.find("lang") != j.end()) {
    lang = j["lang"];
  } else {
    throw std::runtime_error("`lang` value is null");
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

bool rest_server::askTranslate(std::string &text, std::string &tokenized_text, json &output, string &domain, string &lang, bool debugmode)
{
    auto it_translation = std::find_if(_list_translation_model.begin(), _list_translation_model.end(), [&](nmt* l_nmt) 
    {
        return (l_nmt->getDomain() == domain && l_nmt->getLang() == lang);
    }); 
    if (it_translation == _list_translation_model.end())
    {
        cerr << "[ERROR]\t" << currentDateTime() << "\tRESPONSE\t" << "`domain` value is not valid ("+domain+") for TRANSLATION" << endl;
        return false;
    }
    
    tokenized_text = (*it_translation)->tokenize_str(text);
    std::vector<std::string> tokenized_vec = (*it_translation)->tokenize(text);

    vector<vector<string> > tokenized_batched ;
    vector<string> tokenized_vec_tmp;
    for (int l_inc=0; l_inc < (int)tokenized_vec.size(); l_inc++)
    {
        tokenized_vec_tmp.push_back(tokenized_vec[l_inc]);
        if (l_inc == (int)tokenized_vec.size()-1 || ((int)tokenized_vec[l_inc].size() == 1 && (tokenized_vec[l_inc].compare(".")==0 || tokenized_vec[l_inc].compare("\n")==0)))
        {
            tokenized_batched.push_back(tokenized_vec_tmp);
            tokenized_vec_tmp.clear();
        }
    }
    if (_debug_mode != 0 ) cerr << "LOG: "<< currentDateTime() << "\t" << "BATCH SIZE:\t" << (int)tokenized_batched.size() << endl;
    if ((int)tokenized_vec_tmp.size() > 0)
    {
        tokenized_batched.push_back(tokenized_vec_tmp);
    }    
    return askTranslate(tokenized_batched, output, domain, lang, debugmode);
}

bool rest_server::askTranslate(vector<vector<string> > &input, json &output, string &domain, string &lang, bool debugmode)
{
    vector<vector<string> > result_batched ;
    auto it_translation = std::find_if(_list_translation_model.begin(), _list_translation_model.end(), [&](nmt* l_nmt) 
    {
        return (l_nmt->getDomain() == domain && l_nmt->getLang() == lang);
    }); 
    if (it_translation == _list_translation_model.end())
    {
        cerr << "[ERROR]\t" << currentDateTime() << "\tRESPONSE\t" << "`domain` value is not valid ("+domain+") for TRANSLATION" << endl;
        return false;
    }
    (*it_translation)->NMTTranslate(input,result_batched);
    
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
            bool begin_tag=0;
            curr_tag=result_batched.at(i).at(j);
            if (curr_tag.find("I-") == 0)
            {
                curr_tag=curr_tag.substr(2);
            }
            if (curr_tag.find("B-") == 0)
            {
                begin_tag=1;
                curr_tag=curr_tag.substr(2);
            }
            j_tmp.push_back(nlohmann::json::object_t::value_type(string("word"), input.at(i).at(j) ));
            j_tmp.push_back(nlohmann::json::object_t::value_type(string("tag"), result_batched.at(i).at(j) ));
            j_tmp.push_back(nlohmann::json::object_t::value_type(string("value"), string("")));
            i_tmp.push_back(j_tmp);
            if ((int)prev_tag.length() > 0 && (prev_tag.compare(curr_tag) != 0 || begin_tag))
            {
                t_tmp.push_back(nlohmann::json::object_t::value_type(string("phrase"), word_concat ));
                t_tmp.push_back(nlohmann::json::object_t::value_type(string("tag"), prev_tag ));
                json j_value({});
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
        t_tmp.push_back(nlohmann::json::object_t::value_type(string("value"), j_value));
        k_tmp.push_back(t_tmp);
    }
    if (debugmode) output.push_back(nlohmann::json::object_t::value_type(string("TRANSLATION_DEBUG"), i_tmp));
    output.push_back(nlohmann::json::object_t::value_type(string("TRANSLATION"), k_tmp));
    return true;
}


void rest_server::doAuth(const Rest::Request &request,
                         Http::ResponseWriter response) {
  printCookies(request);
  response.cookies().add(Http::Cookie("lang", "fr-FR"));
  response.send(Http::Code::Ok);
}
