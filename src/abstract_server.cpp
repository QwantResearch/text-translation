// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "abstract_server.h"

AbstractServer::AbstractServer(std::string& model_config_path, std::string& tfserving_host, int num_port, int debug_mode){
  _debug_mode = debug_mode;
  _num_port = num_port;
  _model_config_path = model_config_path;
  _tfserving_host = tfserving_host;

  _nlu = make_shared<nlu>(_debug_mode, _model_config_path, _tfserving_host);
}