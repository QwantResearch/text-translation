// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "grpc_route_nmt_impl.h"

grpc::Status GrpcRouteNMTImpl::GetDomains(grpc::ServerContext* context,
                                               const Empty* request,
                                               Domains* response) {

  for (auto& it: _nmt->getDomains()) {
    response->add_domains(it);
  }
  if (_debug_mode)
    cerr << "[DEBUG]: " << currentDateTime() << "\t" << "GetDomains" << endl;
  return grpc::Status::OK;
}

grpc::Status GrpcRouteNMTImpl::GetNMT(grpc::ServerContext* context,
                                               const TextToParse* request,
                                               TextParsed* response) {

  if (_debug_mode) {
    cerr << "[DEBUG]: " << currentDateTime() << "\t" << "GetNMT";
    cerr << "\t" << request->text() << "\t";
  }

  PrepareOutput(request, response);

  std::string text = response->text();
  std::string domain = response->domain();
  std::string lang = response->lang();
  std::string tokenized;

  std::string tokenized_text = _nmt->tokenize_str(text, lang);
  std::vector<std::string> tokenized_vec = _nmt->tokenize(text, lang);

  vector<vector<string> > tokenized_batched;
  tokenized_batched.push_back(tokenized_vec);
 
  std::vector<std::vector<std::string>> output_batch_tokens;
  Status status = _nmt->NMTBatch(tokenized_batched, output_batch_tokens, domain);
  if (!status.ok()){
    return status;
  }
  
  response->set_tokenized(tokenized_text);

  SetOutput(response, tokenized_batched, output_batch_tokens);

  return grpc::Status::OK;
}

grpc::Status GrpcRouteNMTImpl::StreamNMT(grpc::ServerContext* context,
                                                   grpc::ServerReaderWriter< TextParsed,
                                                                            TextToParse>* stream) {
  TextToParse request;
  while (stream->Read(&request)) {

    if (_debug_mode) {
      cerr << "[DEBUG]: " << currentDateTime() << "\t" << "StreamNMT";
      cerr  << "\t" << request.text() << "\t";
    }

    TextParsed response;
    PrepareOutput(&request, &response);

    std::string text = response.text();
    std::string domain = response.domain();
    std::string lang = response.lang();
    std::string tokenized;


    std::string tokenized_text = _nmt->tokenize_str(text, lang);
    std::vector<std::string> tokenized_vec = _nmt->tokenize(text, lang);

    vector<vector<string> > tokenized_batched;
    tokenized_batched.push_back(tokenized_vec);
  
    std::vector<std::vector<std::string>> output_batch_tokens;
    Status status =_nmt->NMTBatch(tokenized_batched, output_batch_tokens, domain);
    if (!status.ok()){
      return status;
    }

    response.set_tokenized(tokenized_text);

    SetOutput(&response, tokenized_batched, output_batch_tokens);

    stream->Write(response);
  }

  return grpc::Status::OK;
}

void GrpcRouteNMTImpl::PrepareOutput(const TextToParse* request, TextParsed* response) {
  std::string text = request->text();

  response->set_text(request->text());
  response->set_domain(request->domain());
  response->set_count(request->count());
  response->set_lang(request->lang());
}

void GrpcRouteNMTImpl::SetOutput(
  TextParsed* response, 
  std::vector<std::vector<std::string>>& tokenized_batch, 
  std::vector<std::vector<std::string>>& output_batch_tokens) {
  for (int i = 0; i < tokenized_batch.size(); i++) {
    std::vector<std::string> tokenized = tokenized_batch[i];
    Tag *tag;
    for (int j = 0; j < tokenized.size(); j++) {
      std::string current_word = tokenized[j];

      std::string current_bio = output_batch_tokens[i][j];
      std::string current_tag = regex_replace(current_bio, regex("^(B-|I-)"), "");
      // Not all NMT models are BIO, current_tag may equal current_bio

      if (j == 0 || current_bio.find("B-") == 0 || (current_tag.compare(tag->tag()) != 0)) { 
        // either a "B-" or a tag that follows a different tag: we create a new tag
        tag = response->add_tag();
        tag->set_phrase(current_word);
        tag->set_tag(current_tag);
      } else { // we have a tag that follows another same tag or a "I": we append text to phrase
        tag->set_phrase(tag->phrase() + " " + current_word);
      }
    }
  }
} 

GrpcRouteNMTImpl::GrpcRouteNMTImpl(shared_ptr<nmt> nmt_ptr, int debug_mode) {
  _nmt = nmt_ptr;
  _debug_mode = debug_mode;
}
