// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include <getopt.h>
#include <iostream>
#include <string>
#include <cstdlib>

#include "rest_server.h"
#include "grpc_server.h"

// default values
int num_port = 9009;
int threads = 1;
int debug = 0;
std::string model_config_path("");
int server_type = 0; // 0 -> REST, 1 -> GRPC
std::string server_host("");
std::string sentencepiece_model_filename("");
std::string source_language("");
std::string target_language("");
bool tensorflow_serving_type=false;
bool set_envvar[10]={0,0,0,0,0,0,0,0,0,0};

void usage() {
  cout << "./text-translation [--threads <nthreads>] [--port <port>] [--grpc] [--tensorflow_serving] --spm <filename> --src <isolang> --tgt <isolang>"
          "[--debug] --model_config_path <filename> --server_host <address:port>\n\n"
          "\t--threads (-t)            number of threads (default 1)\n"
          "\t--port (-p)               port to use (default 9009)\n"
          "\t--grpc (-g)               use grpc service instead of rest\n"
          "\t--debug (-d)              debug mode (default false)\n"
          "\t--src (-i)                source language\n"
          "\t--tgt (-o)                target language\n"
          "\t--spm (-m)                sentencepiece model filename\n"
          "\t--model_config_path (-c)  model_config_path file in which API configuration is set (needed)\n"
          "\t--server_host (-s)        TFServing host (needed)\n"
          "\t--tensorflow_serving (-f) Ask tensorflow serving (marian translation server is default)\n"
          "\t--help (-h)               Show this message\n"
       << endl;
  exit(1);
}

void ProcessArgs(int argc, char **argv) {
  const char *const short_opts = "m:i:o:p:t:c:s:dhgf";
  const option long_opts[] = {
      {"spm", 1, nullptr, 'm'},
      {"src", 1, nullptr, 'i'},
      {"tgt", 1, nullptr, 'o'},
      {"port", 1, nullptr, 'p'},
      {"threads", 1, nullptr, 't'},
      {"model_config_path", 1, nullptr, 'c'},
      {"server_host", 1, nullptr, 's'},
      {"debug", 0, nullptr, 'd'},
      {"help", 0, nullptr, 'h'},
      {"grpc", 0, nullptr, 'g'},
      {"tensorflow_serving", 0, nullptr, 'f'},
      {nullptr, 0, nullptr, 0}};

  while (true) {
    const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

    if (-1 == opt)
      break;

    switch (opt) {
    case 't':
      if (set_envvar[0]==1)
      {
          cout << "[INFO]\t" << currentDateTime() << "\tErasing previous value of threads ("<< threads <<"), given by environment variable, with "<< optarg << endl;
      }
      threads = atoi(optarg);
      break;

    case 'p':
      if (set_envvar[1]==1)
      {
          cout << "[INFO]\t" << currentDateTime() << "\tErasing previous value of port ("<< num_port <<"), given by environment variable, with " << optarg << endl;
      }
      num_port = atoi(optarg);
      break;

    case 'g':
      if (set_envvar[2]==1)
      {
          cout << "[INFO]\t" << currentDateTime() << "\tErasing previous value of gRPC ("<< server_type <<"), given by environment variable, with " << 1 << endl;
      }
      server_type = 1;
      break;

    case 'd':
      if (set_envvar[3]==1)
      {
          cout << "[INFO]\t" << currentDateTime() << "\tErasing previous value of debug ("<< debug <<"), given by environment variable, with " << 1 << endl;
      }
      debug = 1;
      break;

    case 'c':
      if (set_envvar[4]==1)
      {
          cout << "[INFO]\t" << currentDateTime() << "\tErasing previous value of config filename ("<< model_config_path <<"), given by environment variable, with " << optarg << endl;
      }
      model_config_path = optarg;
      break;

    case 's':
      if (set_envvar[5]==1)
      {
          cout << "[INFO]\t" << currentDateTime() << "\tErasing previous value of server_host ("<< server_host <<"), given by environment variable, with " << optarg << endl;
      }
      server_host = optarg;
      break;

    case 'm':
      if (set_envvar[6]==1)
      {
          cout << "[INFO]\t" << currentDateTime() << "\tErasing previous value of sentencepiece model filemane ("<< sentencepiece_model_filename <<"), given by environment variable, with " << optarg << endl;
      }
      sentencepiece_model_filename = optarg;
      break;

    case 'i':
      if (set_envvar[7]==1)
      {
          cout << "[INFO]\t" << currentDateTime() << "\tErasing previous value of source language ("<< source_language <<"), given by environment variable, with " << optarg << endl;
      }
      source_language = optarg;
      break;

    case 'o':
      if (set_envvar[8]==1)
      {
          cout << "[INFO]\t" << currentDateTime() << "\tErasing previous value of target language ("<< target_language <<"), given by environment variable, with " << optarg << endl;
      }
      target_language = optarg;
      break;

    case 'f':
      if (set_envvar[9]==1)
      {
          cout << "[INFO]\t" << currentDateTime() << "\tErasing previous value of tensorflow_serving_type ("<< tensorflow_serving_type <<"), given by environment variable, with " << optarg << endl;
      }
      tensorflow_serving_type = true;
      break;

    case 'h': // -h or --help
    case '?': // Unrecognized option
    default:
      usage();
      break;
    }
  }
  if (model_config_path == "" || server_host == "") {
    cerr << "[ERROR]\t" << currentDateTime() << "\tError, you must set a model_config_path "
         << "and a server_host" << endl;
    usage();
    exit(1);
  }
}

int main(int argc, char **argv) {
  if (getenv("API_NMT_THREADS") != NULL) 
  { 
      threads=atoi(getenv("API_NMT_THREADS")); 
      set_envvar[0]=1; 
      cout << "[INFO]\t" << currentDateTime() << "\tSetting the threads value to "<< threads <<", given by environment variable." << endl;
  }
  if (getenv("API_NMT_PORT") != NULL) 
  { 
      num_port=atoi(getenv("API_NMT_PORT")); 
      set_envvar[1]=1; 
      cout << "[INFO]\t" << currentDateTime() << "\tSetting the port value to "<< num_port <<", given by environment variable." << endl;
  }
  if (getenv("API_NMT_GRPC") != NULL) 
  { 
      server_type = atoi(getenv("API_NMT_GRPC")); 
      set_envvar[2]=1; 
      cout << "[INFO]\t" << currentDateTime() << "\tSetting the gRPC value to "<< server_type <<", given by environment variable." << endl;
  }
  if (getenv("API_NMT_DEBUG") != NULL) 
  { 
      debug = atoi(getenv("API_NMT_DEBUG")); 
      set_envvar[3]=1; 
      cout << "[INFO]\t" << currentDateTime() << "\tSetting the debug value to "<< debug <<", given by environment variable." << endl;
  }
  if (getenv("API_NMT_CONFIG") != NULL) 
  { 
      model_config_path=getenv("API_NMT_CONFIG"); 
      set_envvar[4]=1; 
      cout << "[INFO]\t" << currentDateTime() << "\tSetting the config filename value to "<< model_config_path <<", given by environment variable." << endl;
  }
  if (getenv("API_NMT_SERVER_HOST") != NULL) 
  { 
      server_host=getenv("API_NMT_SERVER_HOST"); 
      set_envvar[5]=1; 
      cout << "[INFO]\t" << currentDateTime() << "\tSetting the TFServing host value to "<< server_host <<", given by environment variable." << endl;
  }

  if (getenv("API_NMT_SENTENCE_PIECE_MODEL") != NULL) 
  { 
      sentencepiece_model_filename=getenv("API_NMT_SENTENCEPIECE_MODEL"); 
      set_envvar[6]=1; 
      cout << "[INFO]\t" << currentDateTime() << "\tSetting the SentencePiece model filename value to "<< sentencepiece_model_filename <<", given by environment variable." << endl;
  }

  if (getenv("API_NMT_SOURCE_LANGUAGE") != NULL) 
  { 
      source_language=getenv("API_NMT_SOURCE_LANGUAGE"); 
      set_envvar[7]=1; 
      cout << "[INFO]\t" << currentDateTime() << "\tSetting the Source Language value to "<< source_language <<", given by environment variable." << endl;
  }

  if (getenv("API_NMT_TARGET_LANGUAGE") != NULL) 
  { 
      target_language=getenv("API_NMT_TARGET_LANGUAGE"); 
      set_envvar[8]=1; 
      cout << "[INFO]\t" << currentDateTime() << "\tSetting the Target Language value to "<< target_language <<", given by environment variable." << endl;
  }

  if (getenv("API_NMT_TENSORFLOW_SERVING") != NULL) 
  { 
      tensorflow_serving_type=getenv("API_NMT_TENSORFLOW_SERVING"); 
      set_envvar[9]=1; 
      cout << "[INFO]\t" << currentDateTime() << "\tSetting the Target tensorflow_serving_type value to "<< tensorflow_serving_type <<", given by environment variable." << endl;
  }

  ProcessArgs(argc, argv);

  cout << "[INFO]\t" << currentDateTime() << "\tCores = " << hardware_concurrency() << endl;
  cout << "[INFO]\t" << currentDateTime() << "\tUsing " << threads << " threads" << endl;
  cout << "[INFO]\t" << currentDateTime() << "\tUsing port " << num_port << endl;
  cout << "[INFO]\t" << currentDateTime() << "\tUsing model config path " << model_config_path << endl;
  cout << "[INFO]\t" << currentDateTime() << "\tUsing translation server host " << server_host << endl;
  cout << "[INFO]\t" << currentDateTime() << "\tUsing Source Language " << source_language << endl;
  cout << "[INFO]\t" << currentDateTime() << "\tUsing Target Language " << target_language << endl;
  cout << "[INFO]\t" << currentDateTime() << "\tUsing sentencepiece model " << sentencepiece_model_filename << endl;
  cout << "[INFO]\t" << currentDateTime() << "\tUsing tensorflow serving " << tensorflow_serving_type << endl;

  unique_ptr<AbstractServer> nmt_api;

  if (server_type == 0) {
    cout << "[INFO]\t" << currentDateTime() << "\tUsing REST API" << endl;
    nmt_api = std::unique_ptr<rest_server>(new rest_server(model_config_path, server_host, sentencepiece_model_filename, source_language, target_language, num_port, tensorflow_serving_type, debug));
  } else {
    cout << "[INFO]\t" << currentDateTime() << "\tUsing gRPC API" << endl;
    nmt_api = std::unique_ptr<grpc_server>(new grpc_server(model_config_path, server_host, sentencepiece_model_filename, source_language, target_language, num_port, tensorflow_serving_type, debug));
  }
  nmt_api->init(threads); //TODO: Use threads number
  nmt_api->start();
  nmt_api->shutdown();
}
