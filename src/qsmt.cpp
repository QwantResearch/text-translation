#include "qsmt.h"

using namespace std;
using namespace nlohmann;
using namespace boost::asio::ip;

bool qsmt::getLocal()
{
    return _local;
}


qsmt::qsmt(string& address, string& iport, string& route, string& name)
{
    _address=address; // e.g.: localhost:8888/tranlation
    _iport=iport;
    _route=route;
    _local=0;
    _model_name=name;
}
qsmt::qsmt()
{
    _address="";
    _iport="";
    _route="";
    _local=1;
    _model_name="None";
}
void qsmt::setqsmt(string& address, string& iport, string& route, string& name)
{
    _address=address; // e.g.: localhost:8888/tranlation
    _iport=iport; // e.g.: 8888
    _route=route; // e.g.: tranlation
    _local=0;
    _model_name=name;
}
bool qsmt::SMTBatch(std::string& tokens, std::string& output_tokens, float& output_scores)
{
    try
    {    
        boost::asio::io_service io_service;
        json j_tmp;// = json::array();
        j_tmp.push_back(json::object_t::value_type(string("text"), tokens ));
        j_tmp.push_back(json::object_t::value_type(string("nbest"), 1 ));
        j_tmp.push_back(json::object_t::value_type(string("source"), string("fr") ));
        j_tmp.push_back(json::object_t::value_type(string("target"), string("en") ));
        std::string s=j_tmp.dump();
        

        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(_address, _iport);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);

        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "POST /"<< _route<<" HTTP/1.0\r\n";
        request_stream << "Host: "<<_address<<"\r\n";
        request_stream << "Content-Type: application/json; charset=utf-8 \r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Content-Length: " << s.size() << "\r\n";    
        request_stream << "Connection: close\r\n\r\n";
        cerr << s <<endl;
        cerr << j_tmp.size() << endl;
        cerr << s.size() << endl;
        request_stream << s;

        // Send the request.
        boost::asio::write(socket, request);

        // Read the response status line. The response streambuf will automatically
        // grow to accommodate the entire line. The growth may be limited by passing
        // a maximum size to the streambuf constructor.
        boost::asio::streambuf response;
//        boost::asio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
//         std::getline(response_stream, status_message);
        stringstream ss;

        boost::system::error_code error;
        while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
        {
            ss << &response;
        }
        socket.close();
        if (error != boost::asio::error::eof)
        {
            throw boost::system::system_error(error);
            return 0;
        }
        string sstr=ss.str();
        string resp_content = sstr.substr(sstr.find("{"),sstr.find("{")-sstr.rfind("}"));
        cout << resp_content;
        cout << endl;
        nlohmann::json j_resp = nlohmann::json::parse(resp_content);
        nlohmann::json l_res;
        if (j_resp.find("result") != j_resp.end())
        {
            l_res=j_resp["result"]; 
            if (l_res.find("text") != l_res.end())
            {
                string tmp=l_res["text"].dump();
                output_tokens=tmp.substr(1,(int)tmp.size()-2);
                output_scores=1.0;
            }
            else
            {
                output_tokens="";
                output_scores=0.0;
            }
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << "\n";
    }
    return 1;
}


// Tanslates a batch of tokenizes sentences.
bool qsmt::NMTBatchSMT(vector< vector< string > > batch_tokens, vector< vector< string > >& output_batch_tokens, vector< float >& output_batch_scores)
{
  // Pad batch.
  std::string linput;
  std::string loutput;
  vector<std::string> vloutput;
  float lscores;
  for (int i=0; i< (int)batch_tokens.size(); i++)
  {
      linput=Join(batch_tokens.at(i)," ");
      loutput="";
      vloutput.clear();
      lscores=0.0;
      SMTBatch(linput,loutput,lscores);
      Split(loutput,vloutput," ");
      if (i == 0)
      {
          output_batch_scores.push_back(lscores/(float)batch_tokens.size());
          output_batch_tokens.push_back(vloutput);
      }
      else
      {
          output_batch_scores.at(0)=output_batch_scores.at(0)+lscores/(float)batch_tokens.size();
          output_batch_tokens.at(0).insert(output_batch_tokens.at(0).end(), vloutput.begin(), vloutput.end());
      }
  }
  return true;
}
