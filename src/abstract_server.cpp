// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "abstract_server.h"

AbstractServer::AbstractServer(std::string& model_config_path, std::string& tfserving_host, std::string& spm_model_filename, std::string& lang_src, std::string& lang_tgt, int num_port, bool tensorflow_serving_type, int debug_mode){
  _debug_mode = debug_mode;
  _num_port = num_port;
  _model_config_path = model_config_path;
  _tfserving_host = tfserving_host;

  _nmt = make_shared<nmt>(_model_config_path, _tfserving_host, spm_model_filename, lang_src,lang_tgt,tensorflow_serving_type);
}


AbstractServer::AbstractServer(std::string &config_file,  int num_port, int debug) 
{
    _debug_mode = debug;
    _num_port = num_port;
//     _model_config_path = model_config_path;
//     _tfserving_host = tfserving_host;
    std::string line;
    YAML::Node config;

    try 
    {
    // Reading the configuration file for filling the options.
        config = YAML::LoadFile(config_file);
        cout << "[INFO]\tDomain\t\tLocation/filename\t\tlanguage"<< endl;
//        _num_port =  config["port"].as<int>() ;
        YAML::Node modelconfig = config["models"]; 
        for (const auto& modelnode: modelconfig)
        {
            std::string domain=modelnode.first.as<std::string>();
            YAML::Node modelinfos = modelnode.second;
            std::string nmt_model_end_point="ws://"+modelinfos["nmt_model_address"].as<std::string>()+":"+modelinfos["nmt_model_port"].as<std::string>()+"/translate";
            std::string spm_model_filename=modelinfos["spm_model"].as<std::string>();
            std::string lang_src=modelinfos["source_language"].as<std::string>();
            std::string lang_tgt=modelinfos["target_language"].as<std::string>();
            try 
            {
                nmt* nmt_pointer = new nmt(domain, nmt_model_end_point, spm_model_filename, lang_src,lang_tgt);
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

}
