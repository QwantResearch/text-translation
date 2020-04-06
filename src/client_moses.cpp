// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

// #include <cstdlib>
#include <iostream>
#include <cstdlib>
#include <string>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using namespace nlohmann;
using namespace boost::asio::ip;

int main(int argc, char **) {

    if (argc-1 > 0) {
        cerr << "This program has no arguments" << endl;
        exit(1);
    }

    string _address("localhost");
    string _iport("8888");
    string tokens("This is a test !");
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
        request_stream << "POST /translation HTTP/1.0\r\n";
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
		ss << &response;
	string sstr=ss.str();
        string resp_content = sstr.substr(sstr.find("{"),sstr.find("{")-sstr.rfind("}"));
        cout << resp_content;
	cout << endl;
// 	std::cout << "END RESPONSE 2" << endl;
        if (error != boost::asio::error::eof)
          throw boost::system::system_error(error);
//         else
//           cerr << "end of stream" <<endl;
	socket.close();
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << "\n";
    }
}
