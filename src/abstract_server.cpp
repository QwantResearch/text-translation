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
    cout << "[INFO]\t" << currentDateTime() << "\tConfig file:\t" << config_file << endl;
    cout << "[INFO]\t" << currentDateTime() << "\tPort number:\t" << _num_port << endl;
    cout << "[INFO]\t" << currentDateTime() << "\tDebug mode:\t" << _debug_mode << endl;
    std::string line;
    YAML::Node config;
    try 
    {
    // Reading the configuration file for filling the options.
        config = YAML::LoadFile(config_file);
        cout << "[INFO]\t" << currentDateTime() << "\tDomain\t\tLocation/filename\t\tlanguage"<< endl;
//        _num_port =  config["port"].as<int>() ;
        YAML::Node modelconfig = config["models"]; 
        for (const auto& modelnode: modelconfig)
        {
            //std::string domain=modelnode.first.as<std::string>();
            YAML::Node modelinfos = modelnode.second;
            std::string domain=modelinfos["domain"].as<std::string>();
            std::string nmt_model_end_point="ws://"+modelinfos["nmt_model_address"].as<std::string>()+":"+modelinfos["nmt_model_port"].as<std::string>()+"/translate";
            std::string spm_model_filename=modelinfos["spm_model"].as<std::string>();
            YAML::Node source_languages = modelinfos["source_languages"]; 
            for (std::size_t i=0;i<source_languages.size();i++) 
            {
                std::cerr << source_languages[i].as<string>() << "\n";
            }
            YAML::Node target_languages = modelinfos["target_languages"]; 
            for (std::size_t i=0;i<target_languages.size();i++) 
            {
                std::cerr << target_languages[i].as<string>() << "\n";
            }
//             std::string lang_src=modelinfos["source_languages"].as< std::vector < std::string> >();
//             std::string lang_tgt=modelinfos["target_languages"].as< std::vector < std::string> >();
            std::string processing_lang_src=modelinfos["source_language_processing"].as< std::string >();
            std::string processing_lang_tgt=modelinfos["target_language_processing"].as< std::string > ();
            
            try 
            {
                nmt* nmt_pointer = new nmt(domain, nmt_model_end_point, spm_model_filename, processing_lang_src, processing_lang_tgt, false);
                nmt_pointer->setDebugMode(_debug_mode);
                _list_translation_model.push_back(nmt_pointer);
                cout << "[INFO]\t" << currentDateTime() << "\t" << domain << "\t" <<  nmt_model_end_point << "\t" <<  spm_model_filename << "\t" <<  processing_lang_src << "\t" <<  processing_lang_tgt << "\t===> loaded" << endl;
            } 
            catch (invalid_argument& inarg) 
            {
                cerr << "[ERROR]\t" << currentDateTime() << "\t" << domain << "\t" <<  nmt_model_end_point << "\t" <<  spm_model_filename << "\t" <<  processing_lang_src << "\t" <<  processing_lang_tgt << endl;
                cerr << "[ERROR]\t" << currentDateTime() << "\t" << inarg.what() << endl;
                continue;
            }
        }
    } catch (YAML::BadFile& bf) {
      cerr << "[ERROR]\t" << currentDateTime() << "\t" << bf.what() << endl;
      exit(1);
    }

}
