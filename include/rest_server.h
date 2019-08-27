// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#ifndef __REST_SERVER_H
#define __REST_SERVER_H

#include <algorithm>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <pistache/client.h>
#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <sstream>
#include <time.h>
#include "yaml-cpp/yaml.h"

#include "nmt.h"
#include "tokenizer.h"

using namespace std;
using namespace nlohmann;
using namespace Pistache;

class rest_server {

public:
  rest_server(string &config_file, int &threads, int debug_mode = 0);
  ~rest_server(){httpEndpoint->shutdown();};

  void init();
  void start();
  void shutdown() { httpEndpoint->shutdown();}
  ostream currentDateTime();

private:
  int _debug_mode;
  int _nbr_threads;
  std::vector<nmt *> _list_translation_model;
  std::shared_ptr<Http::Endpoint> httpEndpoint;
  Rest::Router router;
  typedef std::mutex Lock;
  typedef std::lock_guard<Lock> Guard;
  Lock nluLock;

  void setupRoutes();

  void doTranslateGet(const Rest::Request &request,
                           Http::ResponseWriter response);

  void doTranslatePost(const Rest::Request &request,
                            Http::ResponseWriter response);

  void doTranslateBatchPost(const Rest::Request &request,
                                 Http::ResponseWriter response);

  void fetchParamWithDefault(const json& j, 
                              string& domain, 
                              string& lang, 
                              int& count,
                              float& threshold,
                              bool& debugmode);

  bool askTranslate(std::string &text, std::string &tokenized, json &output, string &domain, string &lang, bool debugmode);
  bool askTranslate(vector<vector<string> > &input, json &output, string &domain, string &lang, bool debugmode);

  void writeLog(string text_to_log) {}

  void doAuth(const Rest::Request &request, Http::ResponseWriter response);
};

#endif // __REST_SERVER_H
